#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cstddef>

#include "ngx_mem_pool.h"

static const uint32_t ngx_pagesize = 4096;
static const uint32_t NGX_MAX_ALLOC_FROM_POOL  = ngx_pagesize - 1;

static const uint32_t NGX_DEFAULT_POOL_SIZE  =  16 * 1024;

static const uint32_t NGX_POOL_ALIGNMENT =  16;

#define NGX_ALIGNMENT sizeof(std::uintptr_t)    /* platform word */

#define ngx_free          free

#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)

#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


using ngx_err_t = int;

#define ngx_close_file close

static const ngx_int_t  NGX_OK       =    0; 
static const ngx_int_t  NGX_ERROR    =   -1;
static const ngx_int_t  NGX_AGAIN    =   -2;
static const ngx_int_t  NGX_BUSY     =   -3;
static const ngx_int_t  NGX_DONE     =   -4;
static const ngx_int_t  NGX_DECLINED =   -5;
static const ngx_int_t  NGX_ABORT    =   -6;


static const ngx_int_t NGX_INVALID_FILE =       -1;
static const ngx_int_t NGX_FILE_ERROR   =       -1;


// 按照 alignment 大小对齐，malloc 分配size大小的内存
void *NgxMemPool::ngx_memalign(size_t alignment, size_t size)
{
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);

    if (err) {
        p = nullptr;
    }

    return p;
}

// 创建内存池
ngx_pool_t* NgxMemPool::ngx_create_pool(std::size_t size)
{
    ngx_pool_t  *p;

    // 以 NGX_POOL_ALIGNMENT(16) 字节对齐，分配 size 大小的内存
    p = (ngx_pool_t *)ngx_memalign(NGX_POOL_ALIGNMENT, size);
    if (p == nullptr) {
        return nullptr;
    }

    // 内存块前部分用于存储 描述信息 ngx_pool_t
    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = nullptr;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    // 小内存块用于分配小于 NGX_MAX_ALLOC_FROM_POOL(4096) 的内存
    // 超过 NGX_MAX_ALLOC_FROM_POOL 大小的内存分配需求，使用大内存块分配
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p; // 记录可用分配的小内存块
    p->large = nullptr;
    p->cleanup = nullptr;

    return p;
}

// 销毁内存池
void NgxMemPool::ngx_destroy_pool()
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;

    // 先析构内存池上分配的对象
    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }

    // 释放大内存块
    // 大内存块的描述结构在小内存块上分配，
    // 所以不需要释放
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

    // 释放所有小内存块
    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_free(p);

        if (n == nullptr) {
            break;
        }
    }
}

// 内存池复位
// WARN: 只释放和重置内存，未调用析构函数
void NgxMemPool::ngx_reset_pool()
{
    ngx_pool_t        *p;
    ngx_pool_large_t  *l;

    // 释放大内存块
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

    // 将小内存块复位
    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(ngx_pool_t);
        p->d.failed = 0;
    }

    pool->current = pool;  // 第一个小内存块空位充足，将第一个内存块作为下次分配的首个内存块
    pool->large = nullptr;
}

// 分配任意大小的内存
void *
NgxMemPool::ngx_palloc(size_t size)
{
    // 若小内存块可以满足需求
    if (size <= pool->max) {
        return ngx_palloc_small(size, 1);
    }

    // 否则使用大内存块分配
    return ngx_palloc_large(size);
}

// 强制在小内存块上分配内存
void *
NgxMemPool::ngx_palloc_small(size_t size, ngx_uint_t align)
{
    u_char      *m;
    ngx_pool_t  *p;

    // 获得最佳小内存块
    p = pool->current;

    do {
        // 获得当前小内存块的空闲空间的起始地址
        m = p->d.last;

        // 确保起始地址按照 NGX_ALIGNMENT( cpu 位宽) 对齐
        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }

        // 如当前内存块有足够的空闲内存
        if ((size_t) (p->d.end - m) >= size) {
            // 将空闲空间起始地址游标偏移，完成内存分配
            p->d.last = m + size;

            return m;
        }

        // 当前内存块空闲内存不够，使用下个内存块
        p = p->d.next;

    } while (p);

    // 所有已有内存块都没有足够的空间
    // 则分配新的内存块
    return ngx_palloc_block(size);
}

// 分配新的完整内存块
void *
NgxMemPool::ngx_palloc_block(size_t size)
{
    u_char      *m;
    size_t       psize;
    ngx_pool_t  *p, *new_;

    // 参照第一个内存的大小，分配新的内存块
    psize = (size_t) (pool->d.end - (u_char *) pool);

    m = (u_char *)ngx_memalign(NGX_POOL_ALIGNMENT, psize);
    if (m == nullptr) {
        return nullptr;
    }

    // 构造新的内存块
    new_ = (ngx_pool_t *) m;

    new_->d.end = m + psize;
    new_->d.next = nullptr;
    new_->d.failed = 0;

    // 第二个内存块开始，只需要 ngx_pool_data_t 
    // 所以空闲的起始地址只需要偏移 ngx_pool_data_t 的大小
    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    new_->d.last = m + size;

    // 之前已有的内存块们都分配失败了，说明内存紧缺，
    // 需要调整选择新的最佳起始小内存块。
    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }

    // 将新加入的内存块加入链表尾部
    p->d.next = new_;

    return m;
}

// 不对齐，malloc 分配 size大小内存
void *
NgxMemPool::ngx_alloc(size_t size)
{
    void  *p;

    p = malloc(size);

    return p;
}

void *
NgxMemPool::ngx_palloc_large(size_t size)
{
    void              *p;
    ngx_uint_t         n;
    ngx_pool_large_t  *large;

    // 不对齐，malloc分配size大小内存
    p = ngx_alloc(size);
    if (p == nullptr) {
        return nullptr;
    }

    n = 0;

    // 尝试找到空闲的 大内存块的管理结构
    for (large = pool->large; large; large = large->next) {
        // 如果找到了，则直接使用来管理新分配的内存
        if (large->alloc == nullptr) {
            large->alloc = p;
            return p;
        }

        // 最多找3次，避免性能消耗太多
        if (n++ > 3) {
            break;
        }
    }

    // 没有空闲的大内存块管理结构，则直接在小内存块上分配
    large = (ngx_pool_large_t *)ngx_palloc_small(sizeof(ngx_pool_large_t), 1);
    if (large == nullptr) {
        ngx_free(p);
        return nullptr;
    }

    // 将新的管理结构加入内存池系统
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

// 和 ngx_palloc 一样，分配时不进行地址对齐
void *
NgxMemPool::ngx_pnalloc(size_t size)
{
    if (size <= pool->max) {
        return ngx_palloc_small(size, 0);
    }

    return ngx_palloc_large(size);
}

// 考虑地址对齐，分配size大小的内存，并初始化为0
void *
NgxMemPool::ngx_pcalloc(size_t size)
{
    void *p;

    p = ngx_palloc(size);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}

// 按 alignment 对齐，强制按分配大内存方式，分配size大小的内存
void *
NgxMemPool::ngx_pmemalign(size_t size, size_t alignment)
{
    void              *p;
    ngx_pool_large_t  *large;

    // 以 alignment 对齐，malloc 分配 size大小的内存
    p = ngx_memalign(alignment, size);
    if (p == nullptr) {
        return nullptr;
    }

    // 从小内存块上分配 大内存块的管理结构
    large = (ngx_pool_large_t *)ngx_palloc_small(sizeof(ngx_pool_large_t), 1);
    if (large == nullptr) {
        ngx_free(p);
        return nullptr;
    }

    // 将malloc分配的内存记录到大内存块的管理结构上，并将管理结构加入内存池系统
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

// 释放所有大内存块
ngx_int_t
NgxMemPool::ngx_pfree(void *p)
{
    ngx_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            ngx_free(l->alloc);
            l->alloc = nullptr;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}

// 分配析构函数的节点，并分配size大小的内存块作为析构函数的参数内存
ngx_pool_cleanup_t *
NgxMemPool::ngx_pool_cleanup_add(size_t size)
{
    ngx_pool_cleanup_t  *c;

    // 在小内存块上分配析构函数管理结构
    c = (ngx_pool_cleanup_t *)ngx_palloc(sizeof(ngx_pool_cleanup_t));
    if (c == nullptr) {
        return nullptr;
    }

    // 分配 size大小的内存块作为析构函数的参数内存
    if (size) {
        c->data = ngx_palloc(size);
        if (c->data == nullptr) {
            return nullptr;
        }

    } else {
        c->data = nullptr;
    }

    c->handler = nullptr;
    c->next = pool->cleanup;

    pool->cleanup = c;

    return c;
}

