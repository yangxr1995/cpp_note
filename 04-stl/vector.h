#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <ostream>
#include <utility>
#include <vector>

using namespace std;

#include "./allocator.h"

template<typename T, typename Alloca = Allocator<T>>
class vector {
public:
	vector(unsigned int len = 10) {
		_first = _allocator.allocate(len);
		_last = _first;
		_end = _first + len;
	}

	vector(const vector<T> &x) {
		T *p, *p2;
		unsigned int cap, n;

		cap = x._end - x._first;
		n = x.size();

		_first = _allocator.allocate(cap);
		_end = _first + cap;
		_last = _first + n;

		for (unsigned int i = 0; i < n; i++) {
			_allocator.construct(_first + i, x._first[i]);
		}
	}

	~vector() {
		for (T *p = _first; p < _last; p++) {
			_allocator.destroy(p);
		}
		_allocator.deallocate(_first, 1);
		_first = _last = _end = nullptr;
	}

	vector<T> &operator=(const vector<T> &v) {
		for (T *p = _first; p < _last; p++) {
			_allocator.destruct(p);
		}
		_allocator.deallocate(_first);

		T *p, *p2;
		unsigned int cap, n;

		cap = v._end - v._first;
		n = v._last - v._first;

		_first = _allocator.allocate(cap);
		_end = _first + cap;
		_last = _first + v.size();

		for (unsigned int i = 0; i < n; i++) {
			_allocator.construct(_first + i, v._first[i]);
		}

		return *this;
	}

	template<typename Ty>
	void push_back(Ty &&val) {
		if (full())
			expand();
		_allocator.construct(_last, std::forward<Ty>(val));	
		_last++;
	}

	void pop_back() {
		if (empty())
			return;
		_last--;
		_allocator.destruct(_last);
	}

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (full())
            expand();
        _allocator.construct(_last, std::forward<Args>(args)...);
        ++_last;
    }

	// 对于不修改私有成员的方法，都用const修饰 this
	// 方便传入 const vector<T>
	// 返回匿名对象，母函数接受匿名对象的方法
	// const T &a = b.back(); // 匿名对象为常量，只能用常引用
	// T a = b.back(); // 母函数分配a的内存，并传递给back进行匿名对象的拷贝构造
	T back() const {
		return _last[-1];
	}

	bool empty() const {
		return _last <= _first ? true : false;
	}

	bool full() const {
		return _last >= _end ? true : false;
	}

	unsigned int size() const {
		// 注意：cpp 两个指针相减和类型相关
		// 实际得到的是 [_first, _last) 的 T类型的元素数量
		// 和 ((char *)_last - (char *)_first) / sizeof(T) 不同
		return (_last - _first);
	}

    // 迭代器实现有问题，无法兼容
    // copy(v.begin(), v.end(), ostream_iterator(cout, " "));
	class iterator {
	public:
		iterator(T *p)
		:_p(p)
		{}

		bool operator!=(const iterator &it) const
		{
			return _p != it._p;
		}

		iterator &operator++() 
		{
			_p++;
			return *this;
		}

		T operator*() {
			return *_p;
		}

		const T operator*() const {
			return *_p;
		}

	private:
		T *_p;
	};

	iterator begin() const {
		return iterator(_first);
	}

	iterator end() const {
		return iterator(_last);
	}

private:
	void expand() {
		unsigned int cap, n;
		T *tmp, *p1, *p2;
		
		cap = (_end - _first) * 2;
		n = _last - _first;

		cout << "expand to : " << cap << endl;
		tmp = _allocator.allocate(cap);

		for (int i = 0; i < n; i++) {
			_allocator.construct(tmp + i, _first[i]);
		}
		_first = tmp;
		_last = _first + n;
		_end = _first + cap;
	}

	Alloca _allocator;
	T *_first;   // 数组地址
	T *_last;    // 末尾有效元素的后一个位置
	T *_end;     // 数组末尾后一个位置
};

