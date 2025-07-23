#ifndef __NGX_MEM_POOL_H__
#define __NGX_MEM_POOL_H__

#include <cstddef>
#include <cstdint>

using ngx_pool_cleanup_pt = void (*)(void *);

// 记录注册的析构函数及其参数
struct ngx_pool_cleanup_t {
    ngx_pool_cleanup_pt   handler; // 同于的析构函数
    void                 *data;    // 析构函数的参数
    ngx_pool_cleanup_t   *next;    // 下一个析构函数
};

// 记录分配大内存块
struct ngx_pool_large_t {
    ngx_pool_large_t     *next;  // 下一个大内存块
    void                 *alloc; // 大内存块的地址
};

using  u_char = unsigned char;

using ngx_uint_t = std::uintptr_t;
using ngx_int_t = std::intptr_t;

struct ngx_pool_t;

// 小内存块的句柄
struct ngx_pool_data_t {
    u_char               *last; // 空闲内存的头部
    u_char               *end;  // 整个内存块的尾部的哨兵
    ngx_pool_t           *next; // 下一个内存块
    uint32_t            failed; // 当前内存块分配内存失败的次数
};

// 内存池的句柄
struct ngx_pool_t {
    ngx_pool_data_t       d;       // 管理一块内存块的描述结构(管理小内存)
    std::size_t           max;     // 一个内存块的大小
    ngx_pool_t           *current; // 空闲内存空链表头(管理小内存)
    ngx_pool_large_t     *large;   // 大内存块的链表头
    ngx_pool_cleanup_t   *cleanup; // 链表头，连接析构函数，用于析构内存池上分配构造的对象
};

class NgxMemPool {
    public:
        NgxMemPool(size_t size)
        : size_(size) {
            pool = ngx_create_pool(size_);
        }

        ~NgxMemPool() {
            ngx_destroy_pool();
        }

        ngx_pool_t *ngx_create_pool(size_t size);
        void ngx_destroy_pool();
        void ngx_reset_pool();

        // 分配任意大小的内存
        void *ngx_palloc(size_t size);
        // 和 ngx_palloc 一样，分配时不进行地址对齐
        void *ngx_pnalloc(size_t size);
        // 考虑地址对齐，分配size大小的内存，并初始化为0
        void *ngx_pcalloc(size_t size);
        // 按 alignment 对齐，强制按分配大内存方式，分配size大小的内存
        void *ngx_pmemalign(size_t size, size_t alignment);
        // 释放所有大内存块
        ngx_int_t ngx_pfree(void *p);
        // 分配析构函数的节点，并分配size大小的内存块作为析构函数的参数内存
        ngx_pool_cleanup_t *ngx_pool_cleanup_add(size_t size);

    private:
        void *ngx_memalign(size_t alignment, size_t size);
        void *ngx_palloc_small(size_t size, ngx_uint_t align);
        void *ngx_palloc_block(size_t size);
        void *ngx_palloc_large(size_t size);
        void *ngx_alloc(size_t size);

    private:
        ngx_pool_t *pool;
        size_t size_;
};


#endif
