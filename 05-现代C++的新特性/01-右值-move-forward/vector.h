#include <algorithm>
#include <iostream>
#include <utility>

class Test {
    public:
        Test() {
            std::cout << "Test()" << std::endl;
        }
        ~Test() {
            std::cout << "~Test()" << std::endl;
        }
        Test(const Test &t) {
            std::cout << "Test(const Test &t)" << std::endl;
        }
};

template <typename T>
struct Allocator {
    T *allocate(size_t size) {
        return (T *)malloc(size * sizeof(T));
    }
    template<typename Ty>
    void construct(T *p, Ty &&val) {
        new (p) T(std::forward<Ty>(val));
    }
    // void construct(T *p, T &&val) {
    //     new (p) T(std::move(val));
    // }
    // void construct(T *p, const T &val) {
    //     new (p) T(val);
    // }
    void deallocate(T *p) {
        free(p);
    }
    void destructor(T *p) {
        p->~T();
    }
};

template <typename T, typename Alloca = Allocator<T>>
class Vector {
    public:
        Vector<T, Alloca>(size_t size = 1) {
            _begin = _alloca.allocate(size);
            _last = _begin;
            _end = _begin + size;
        }
        ~Vector<T, Alloca>() {
            for (T *tmp = _begin; tmp != _last; ++tmp)
                _alloca.destructor(tmp);
            _alloca.deallocate(_begin);
            _begin = _last = _end = nullptr;
        }
        Vector<T, Alloca>(const Vector<T> &v) {
            _begin = _alloca.allocate(v.capacity());
            _last = _begin + v.size();
            _end = _begin + v.capacity();
            for (int i = 0; i < v.size(); ++i)
                _alloca.construct(_begin + i, v._begin[i]);
        }
        size_t size() const {
            return _last - _begin;
        }
        size_t capacity() const {
            return _end - _begin;
        }
        template<typename Ty>
        void push_back(Ty &&a) {
            if (full())
                expand();
            // _alloca.construct(_last, std::forward(a));
            _alloca.construct(_last, std::forward<Ty>(a));
            ++_last;
        }
        // void push_back(T &&a) {
        //     if (full())
        //         expand();
        //     _alloca.construct(_last, std::move(a));
        //     ++_last;
        // }
        // void push_back(const T &a) {
        //     if (full())
        //         expand();
        //     _alloca.construct(_last, a);
        //     ++_last;
        // }
        void pop_back() {
            if (empty()) throw std::out_of_range("Vector is empty");
            --_last;
            _alloca.destructor(_last);
        }
        T back() {
            if (empty()) throw std::out_of_range("Vector is empty");
            return *(_last - 1);
        }
        bool full() const {
            return _last == _end;
        }
        bool empty() const {
            return _begin == _last;
        }

        class iterator {
            public:
                iterator(T *p) :_p(p) {}
                T *operator++() {
                    ++_p;
                    return _p;
                }
                T &operator*() {
                    return *_p;
                }
                T *operator->() {
                    return _p;
                }
                bool operator!=(const iterator &it) const {
                    return _p != it._p;
                }
            private:
                T *_p;
        };

        iterator begin() const {
            return iterator(_begin);
        }

        iterator end() const {
            return iterator(_last);
        }

    private:
        void expand() {
            size_t cap = capacity() * 2;
            T *_new = _alloca.allocate(cap);
            for (int i = 0; i < size(); ++i) {
                _alloca.construct(_new + i, _begin[i]);
                _alloca.destructor(_begin + i);
            }
            _last = _new + size();
            _end = _new + cap;
            _alloca.deallocate(_begin);
            _begin = _new;
        }
        T *_begin;
        T *_last;
        T *_end;
        Alloca _alloca;
};

