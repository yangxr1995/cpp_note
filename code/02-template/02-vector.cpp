#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
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
	void construct(T *addr, const T &a) {
		new (addr) T(a);
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

	void push_back(const T &x) {
		if (full())
			expand();
		_allocator.construct(_last, x);	
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
	//func();

	vector<int>v_int;

	for (int i = 0; i < 10; i++)
		v_int.push_back(i);

	for (auto it = v_int.begin(); it != v_int.end(); ++it)
		cout << *it << " ";
	cout << endl;

	for (int i: v_int)
		cout << i << " ";
	cout << endl;

	vector<int>v = v_int;

	for (int i: v)
		cout << i << " ";
	cout << endl;

	return 0;
}
