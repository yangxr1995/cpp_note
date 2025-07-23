#ifndef __SGI_ALLOCATOR_H__
#define __SGI_ALLOCATOR_H__

#include <algorithm>
#include <atomic>
#include <cstring>
#include <cstddef>
#include <memory>
#include <mutex>
#include <cstdlib>
#include <new>
#include <iostream>

#include <stdatomic.h>
#include <sys/types.h>

#define DELETE_FREE_MALLOC

namespace sgi {

    // 对 malloc/free 的封装，添加了malloc失败后调用 __malloc_alloc_oom_handler 的功能
    template <int __inst>
        class __malloc_alloc_template {

            private:

                static void* _S_oom_malloc(size_t);
                static void* _S_oom_realloc(void*, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
                // 当malloc分配失败时调用，
                // 可用于绑定清理系统内存的函数, 
                // 以实现清理内存后malloc成功
                static void (* __malloc_alloc_oom_handler)();
#endif

            public:

                static void* allocate(size_t __n)
                {
                    void* __result = malloc(__n);
                    if (0 == __result) __result = _S_oom_malloc(__n);
                    return __result;
                }

                static void deallocate(void* __p, size_t /* __n */)
                {
                    free(__p);
                }

                static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
                {
                    void* __result = realloc(__p, __new_sz);
                    if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
                    return __result;
                }

                static void (* __set_malloc_handler(void (*__f)()))()
                {
                    void (* __old)() = __malloc_alloc_oom_handler;
                    __malloc_alloc_oom_handler = __f;
                    return(__old);
                }

        };

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
    template <int __inst>
        void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

    template <int __inst>
        void*
        __malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
        {
            void (* __my_malloc_handler)();
            void* __result;

            for (;;) {
                __my_malloc_handler = __malloc_alloc_oom_handler;
                if (0 == __my_malloc_handler) { throw std::bad_alloc(); }
                (*__my_malloc_handler)();
                __result = malloc(__n);
                if (__result) return(__result);
            }
        }

    template <int __inst>
        void* __malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
        {
            void (* __my_malloc_handler)();
            void* __result;

            for (;;) {
                __my_malloc_handler = __malloc_alloc_oom_handler;
                if (0 == __my_malloc_handler) { throw std::bad_alloc(); }
                (*__my_malloc_handler)();
                __result = realloc(__p, __n);
                if (__result) return(__result);
            }
        }

    typedef __malloc_alloc_template<0> malloc_alloc;



    static const int _ALIGN = 8;        // 最小内存块8字节
    static const int _MAX_BYTES = 128;  // 最大内存块128字节
    static const int _NFREELISTS = 16;  // _S_free_list[] 共 16个元素

    template<bool threads, int inst>
        class sgi_allocator {

            private:
                // 将size对齐8字节，上取整
                static size_t
                    _S_round_up(size_t __bytes) 
                    { return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }

                // 小内存块节点
                union _Obj {
                    union _Obj* _M_free_list_link; // 用于将内存块链接成链表
                    char _M_client_data[1];     // 未使用 /* The client sees this.        */
                };

                static _Obj* volatile _S_free_list[];  // 小内存块链表数组

                // 将 字节数 映射到 _S_free_list[] 的下标
                static  size_t _S_freelist_index(size_t __bytes) {
                    return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
                }

                // 根据分配大小n ，创建一级内存池链表，否则将第一个内存节点给用户
                static void * _S_refill(size_t __n);

                //  分配一级内存节点。
                // @ size  : 内存节点的大小
                // @ nobjs : 输入期望分配的内存节点的数量, 输出实际分配的内存节点数量
                static char* _S_chunk_alloc(size_t __size, int& __nobjs);

                // 维护二级内存池
                static char* _S_start_free;
                static char* _S_end_free;
                static size_t _S_heap_size;

                static atomic_uint _S_inst_nb;

                // 记录从系统分配的堆内存地址
                // 方便内存池析构时释放所有堆内存
                struct heap_addr_node {
                    void *_M_addr; 
                    heap_addr_node *_M_next;
                };

                static heap_addr_node *_S_heap_addr_head;

                static std::mutex _S_node_allocator_lock;

            public:

                sgi_allocator() {
                    _S_inst_nb++;
                }

                ~sgi_allocator() {
                    if (--_S_inst_nb == 0) {
#ifdef DELETE_FREE_MALLOC
                        std::lock_guard<std::mutex> guard(_S_node_allocator_lock);
                        if (_S_heap_addr_head != nullptr) {
                            heap_addr_node *pos, *next;
                            for (pos = _S_heap_addr_head; pos; pos = next) {
                                next = pos->_M_next;
                                free(pos);
                                // malloc_alloc::deallocate(pos, 1);
                            }
                            _S_heap_addr_head = nullptr;
                        }
#endif
                    }

                }

                /* __n must be > 0      */
                static void * allocate(size_t __n)
                {
                    void* __ret = 0;

                    if (__n > (size_t) _MAX_BYTES) { // 如果目标大小大于内存池能处理的最大大小，使用一级空间配置分配
                        __ret = malloc_alloc::allocate(__n);
                    }
                    else {
                        // 根据分配大小，找到对应链表头的地址
                        _Obj* volatile * __my_free_list
                            = _S_free_list + _S_freelist_index(__n);

                        if constexpr (threads) {
                            std::lock_guard<std::mutex> guard(_S_node_allocator_lock);
                            // 如果内存池链表为空，则分配，否则将第一块内存返回
                            _Obj* __result = *__my_free_list;
                            if (__result == 0)
                                __ret = _S_refill(_S_round_up(__n)); // 若一级内存池链表为空，则分配链表，并获得一块从一级内存池分配的节点
                            else {
                                *__my_free_list = __result -> _M_free_list_link; //  若一级内存池链表不为空，则直接分配
                                __ret = __result;
                            }
                        }
                    }

                    return __ret; // 返回分配的内存
                };

                /* __p may not be 0 */
                static void deallocate(void* __p, size_t __n)
                {

                    if (__n > (size_t) _MAX_BYTES)
                        malloc_alloc::deallocate(__p, __n); // 走一级分配器
                    else {
                        _Obj* volatile*  __my_free_list
                            = _S_free_list + _S_freelist_index(__n); // 根据内存大小，获得一级内存池链表头的地址 
                        _Obj* __q = (_Obj*)__p; // 将用户释放的内存，当内存节点使用

                        // acquire lock
                        if constexpr (threads) {
                            std::lock_guard<std::mutex> lock(_S_node_allocator_lock);
                            __q -> _M_free_list_link = *__my_free_list; // 见节点接入对应链表
                            *__my_free_list = __q;
                        }
                        // lock is released here
                    }
                }

                static void * reallocate(void* __p, size_t __old_sz, size_t __new_sz);

        } ;

    template <bool threads, int inst>
        void *
        sgi_allocator<threads, inst>::reallocate(void* __p,
                size_t __old_sz,
                size_t __new_sz)
        {
            void* __result;
            size_t __copy_sz;

            if (__old_sz > (size_t) _MAX_BYTES && __new_sz > (size_t) _MAX_BYTES) {
                return(realloc(__p, __new_sz)); // 内存过大，调用系统的 realloc
            }
            if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p); // 内存不需要重新分配
            __result = allocate(__new_sz);
            __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
            memcpy(__result, __p, __copy_sz); 
            deallocate(__p, __old_sz);
            return(__result);
        }


    // 二级内存池的起始地址
    template <bool threads, int inst>
        char* sgi_allocator<threads, inst>::_S_start_free = 0;

    // 二级内存池的结束地址
    template <bool threads, int inst>
        char* sgi_allocator<threads, inst>::_S_end_free = 0;


    template <bool threads, int inst>
        atomic_uint sgi_allocator<threads, inst>::_S_inst_nb = 0;

    // 记录从系统分配的堆内存地址
    template<bool threads, int inst>
        typename sgi_allocator<threads, inst>::heap_addr_node *
        sgi_allocator<threads, inst>::_S_heap_addr_head = nullptr;

    // 记录一共从系统分配过多少内存
    template <bool threads, int inst>
        size_t sgi_allocator<threads, inst>::_S_heap_size = 0;

    // 一级内存池数组
    template <bool threads, int inst>
        typename sgi_allocator<threads, inst>::_Obj* volatile
        sgi_allocator<threads, inst> ::_S_free_list[
        _NFREELISTS
        ] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

    template<bool threads, int inst>
        std::mutex sgi_allocator<threads, inst>::_S_node_allocator_lock;


    // 多线程场景，假设已经获得了锁
    // 分配一级内存节点数组，并构建为链表
    template <bool threads, int inst>
        void *
        sgi_allocator<threads, inst>::_S_refill(size_t __n)
        {
            int __nobjs = 20;
            char* __chunk = _S_chunk_alloc(__n, __nobjs); // 分配一级内存节点数组，nobjs为数据大小
            _Obj* volatile* __my_free_list;
            _Obj* __result;
            _Obj* __current_obj;
            _Obj* __next_obj;
            int __i;

            if (1 == __nobjs) return(__chunk); // 若只分配到一个节点，直接返回
            __my_free_list = _S_free_list + _S_freelist_index(__n); // 根据分配大小，找到对应内存节点链表头的地址

            // 在内存节点数组上构建内存结点链表
            __result = (_Obj*)__chunk;  // 将第一个节点分配给用户
            *__my_free_list = __next_obj = (_Obj*)(__chunk + __n); // 剩余的节点构成链表
            for (__i = 1; ; __i++) {
                __current_obj = __next_obj;
                __next_obj = (_Obj*)((char*)__next_obj + __n);
                if (__nobjs - 1 == __i) {
                    __current_obj -> _M_free_list_link = 0;
                    break;
                } else {
                    __current_obj -> _M_free_list_link = __next_obj;
                }
            }
            return (__result); //  返回分配的节点
        }

    //  为了防止malloc堆过度碎片化，我们以大块分配内存。
    //  我们假设size已正确对齐。
    //  我们持有内存分配锁。
    //
    //  多线程场景，假设已经获得了锁
    //  分配一级内存节点。
    // @ size  : 内存节点的大小
    // @ nobjs : 输入期望分配的内存节点的数量, 输出实际分配的内存节点数量
    template <bool threads, int inst>
        char*
        sgi_allocator<threads, inst>::_S_chunk_alloc(size_t __size, 
                int& __nobjs)
        {
            char* __result;
            size_t __total_bytes = __size * __nobjs;
            size_t __bytes_left = _S_end_free - _S_start_free; // 二级内存池剩余的内存大小
            bool __is_alloc_from_heap = false;

            if (__bytes_left >= __total_bytes) { // 若二级内存池有足够的内存,则直接从二级内存池中分配
                __result = _S_start_free;         
                _S_start_free += __total_bytes;
                return(__result);
            } else if (__bytes_left >= __size) { // 若二级内存池不够分配整个内存，但够分配至少一个内存节点，仍从二级内存池中分配内存节点
                __nobjs = (int)(__bytes_left/__size); // 计算二级内存池还能分配多少个节点, 返回实际分配的数量
                __total_bytes = __size * __nobjs; 
                __result = _S_start_free;            
                _S_start_free += __total_bytes;
                return(__result); 
            } else {                             // 若二级内存池一个内存节点都无法分配。
                                                 // 只能从系统堆中分配。
                                                 // 为了防止malloc堆过度碎片化。
                                                 // 根据分配大小和以往分配大小增加本次实际分配大小。
                                                 // 以从malloc中分配大块内存。
                size_t __bytes_to_get = 
                    2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
                // Try to make use of the left-over piece.
                if (__bytes_left > 0) { // 如果二级内存池还有内存，则将其全部导入合适的一级内存池中
                    _Obj* volatile* __my_free_list =
                        _S_free_list + _S_freelist_index(__bytes_left);   // 找到对应内存节点链表头的地址

                    ((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
                    *__my_free_list = (_Obj*)_S_start_free;
                }
                // 从heap分配时需要记录分配的地址
#ifdef DELTE_FREE_MALLOC
                __bytes_to_get += sizeof(heap_addr_node);
#endif
                _S_start_free = (char*)malloc(__bytes_to_get); // 二级内存池分配内存，为了防止malloc堆过度碎片化，分配的内存块比较大
                if (0 == _S_start_free) { // 分配失败，系统没有内存
#ifdef DELTE_FREE_MALLOC
                    __bytes_to_get -= sizeof(heap_addr_node); // 从一级内存池分配时，不需要记录地址
#endif
                    size_t __i;           // 尝试从一级内存池其他内存节点链表中获得空闲内存
                    _Obj* volatile* __my_free_list;
                    _Obj* __p;
                    // 尽量利用现有的资源。这不会造成伤害。我们不会尝试更小的请求，因为这在多进程机器上往往会引发灾难。
                    for (__i = __size;
                            __i <= (size_t) _MAX_BYTES;
                            __i += (size_t) _ALIGN) {
                        __my_free_list = _S_free_list + _S_freelist_index(__i); 
                        __p = *__my_free_list; // 获得第一个内存节点
                        if (0 != __p) { // 若第一个内存节点存在
                            *__my_free_list = __p -> _M_free_list_link; // 从链表中删除第一个内存节点
                            _S_start_free = (char*)__p;                 // 将内存节点导入二级内存池
                            _S_end_free = _S_start_free + __i;
                            return(_S_chunk_alloc(__size, __nobjs));    // 再次从二级内存池中分配一级内存池
                                                                        // Any leftover piece will eventually make it to the
                                                                        // right free list.
                        }
                    }
                    // 若一级内存池中没有合适的内存
                    // 只能再次尝试从系统中分配
                    // 不过这次分配再失败会处理异常
                    _S_end_free = 0;	// In case of exception.
                                        // 从heap分配时需要记录分配的地址
#ifdef DELTE_FREE_MALLOC
                    __bytes_to_get += sizeof(heap_addr_node);
#endif

                    _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
                    __is_alloc_from_heap = true; // 若 malloc_alloc::allocate返回，则一定是分配成功
                }
                else {
                    // 若从系统中分配内存成功, 记录下地址
                    __is_alloc_from_heap = true;
                }

                _S_heap_size += __bytes_to_get; // 累加从系统堆中分配的内存大小
                _S_end_free = _S_start_free + __bytes_to_get; // 设置二级内存池

#ifdef DELETE_FREE_MALLOC
                if (__is_alloc_from_heap) { // 分配的内存块首部用于 heap_addr_node
                    heap_addr_node *node = (heap_addr_node *)_S_start_free;
                    node->_M_addr = _S_start_free;
                    node->_M_next = _S_heap_addr_head;
                    _S_heap_addr_head = node;
                    _S_start_free = (char *)_S_start_free + sizeof(heap_addr_node);
                }
#endif

                // 成功获得空闲内存，内存可能来自堆或一级内存池
                return(_S_chunk_alloc(__size, __nobjs)); // 再次从二级内存池中分配一级内存池
            }
        }

    // 中间层，
    // __allocator_base<_Tp> 定义了 construct destory allocate deallocate 等
    // sgi_allocator 只需要定义 allocate deallocate 
    template<typename _Tp, bool threads = false, int inst = 0>
        class allocator : public std::__allocator_base<_Tp> {
            public:
                typedef _Tp        value_type;
                typedef size_t     size_type;
                typedef ptrdiff_t  difference_type;

                // These were removed for C++20.
                typedef _Tp*       pointer;
                typedef const _Tp* const_pointer;
                typedef _Tp&       reference;
                typedef const _Tp& const_reference;

                template<typename _Tp1>
                    struct rebind {
                        typedef allocator<_Tp1, threads, inst> other;
                    };
     
            public:
                _Tp*
                    allocate(size_type __n, const void* = static_cast<const void*>(0)) {
                        _Tp *tmp = static_cast<_Tp *>(_sgi.allocate(__n * sizeof(_Tp)));
                        return tmp;
                    }

                void
                    deallocate(_Tp* __p, size_type __n __attribute__ ((__unused__))) {
                        _sgi.deallocate(__p, __n * sizeof(_Tp));
                    }

            private:
                sgi_allocator<threads, inst> _sgi;
        };

}

#endif
