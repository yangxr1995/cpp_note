#include <iostream>

template <typename _Tp> 
class Allocator : public std::__allocator_base<_Tp> {
    public:

        typedef _Tp        value_type;
        typedef size_t     size_type;
        typedef ptrdiff_t  difference_type;

        typedef _Tp*       pointer;
        typedef const _Tp* const_pointer;
        typedef _Tp&       reference;
        typedef const _Tp& const_reference;

        template<typename _Tp1>
            struct rebind {
                typedef Allocator<_Tp1> other;
            };

        _Tp *allocate(size_type __n, const void* = static_cast<const void*>(0)) {
            return (_Tp *)malloc(sizeof(_Tp) * __n);
        }

        void deallocate(_Tp* __p, size_type __n __attribute__ ((__unused__))) {
            free(__p);
        }

        // template<typename Ty>
        // void construct(_Tp *p, Ty &&val) {
        //     new (p) _Tp(std::forward<Ty>(val));
        // }

        template<typename... Args>
        void construct(_Tp *p, Args&&... args) {
            new (p) _Tp(std::forward<Args>(args)...);
        }

        void destroy(_Tp *p) {
            p->~_Tp();
        }
};


