#include <cstdlib>
#include <iostream>
#include <ostream>
#include <utility>

#include "string.h"

using namespace std;

/*
 * 动态数组
 */

template<typename T>
class Allocator {
public:
	T *allocate(unsigned int n) {
		return (T *)malloc(sizeof(T) * n);
	}
	void deallocate(T *addr) {
		free(addr);
	}
	template <typename Ty>
	void construct(T *addr, Ty &&val) {
		new (addr) T(std::forward<Ty>(val));
	}
	void destruct(T *a) {
		a->~T();
	}
};

/*
 * 定义分配器类型 Alloca ，默认为 Allocator<T>类型
 */
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
			_allocator.destruct(p);
		}
		_allocator.deallocate(_first);
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

	// 代码调用两个函数
	// push_back(const T &val)
	// push_back(T &&val)
	// 所以使用函数模板，生成两份代码
	// 需要记忆的是引用折叠， Ty &&val
	// 如果传递的是 const T &val, 	Ty &&val -> const T &val
	// 如果传递的是       T &&val, 	Ty &&val ->       T &&val
	template<typename Ty>
	void push_back(Ty &&val) {
		if (full())
			expand();
		// 不论是 const T &val
		//              T &&val
		// val 本身是左值，那么连接 construct(T *, const Ty &val)
		// 为了当 val为右值时连接   construct(T *, Ty &&val)
		// 就需要将 val转换为右值
		// 使用  std::forward<T>(val)  完美类型转发
		// 当val引用的是左值时，val转换为左值
		// 当val引用的是右值时，val转换为右值
		_allocator.construct(_last, std::forward<Ty>(val));	
		_last++;
	}

	void pop_back() {
		if (empty())
			return;
		_last--;
		_allocator.destruct(_last);
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
		return iterator(_end);
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

class test {
public:
	test(int a = 1, int b =2)
		:_a(a), _b(b)
	{
	}
	
	~test() {
	}

	friend ostream &operator<<(ostream &out, test &t);
	friend void func2(test &t);

	int _a;
	int _b;
};

void func2(test &t)
{

}

ostream &operator<<(ostream &out, const test &t)
{
	out << "a : " << t._a << ", b : " << t._b;
	return out;
}

void func()
{
	vector<test> v;

	for (int i = 0; i < 20; i++)
		v.push_back(test(i, i + 1));

	vector<test> v2;
	v2 = v;
	const test &t1 = v2.back();


	for (int i = 0; i < 20; i++) {
		cout << i << " : " << v2.back() << endl;
		v2.pop_back();
	}

}

int main()
{
	vector<String> v;

	String s1("222");
	v.push_back(s1); // vector::push_back(const T &val)
	v.push_back(String("111")); // vector::push_back(T &&val)

	return 0;
}
