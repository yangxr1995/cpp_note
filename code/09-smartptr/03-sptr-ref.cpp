#include <iostream>
#include <cassert>
using namespace std;

/*
 * 实现带参数的智能指针
 */

template<typename T>
class SmartPtr {
	public:
		SmartPtr(T *p = nullptr)
		:_p(p) {
			_refcnt = new RefCnt(p);
		}

		SmartPtr(const SmartPtr &a) {
			_p = a._p;
			_refcnt = a._refcnt;
			_refcnt->add();
		}

		SmartPtr &operator=(SmartPtr &&a) {
			_p = a._p;
			if (_refcnt->del() == 0)
				delete _refcnt;	
			_refcnt = a._refcnt;
			_refcnt->add();
			return *this;
		}

		T &operator*() {
			return *_p;
		}

		T *operator->() {
			return _p;	
		}

		~SmartPtr() {
			if (_refcnt->del() == 0)
				delete _refcnt;
		}

		class RefCnt {
			public:
				RefCnt(T *addr = nullptr) {
					_addr = addr;
					if (addr != nullptr)		
						_cnt = 1;
				}

				~RefCnt() {
					if (_addr != nullptr)
						delete _addr;
				}

				void add() {
					if (_addr != nullptr)
						_cnt++;
				}

				int del() {
					if (_addr != nullptr)
						return --_cnt;
					return 0;
				}

			private:	
				int _cnt;
				T *_addr;
		};

	private:
		T *_p;
		RefCnt *_refcnt;
};

class Test {
	public:	
		void test() {
			cout << "test" << endl;
		}
};

SmartPtr<int> test() {

	SmartPtr<int> sp(new int);
	SmartPtr<int> sp2(sp);
	*sp = 10;
	cout << "*sp " << *sp << endl;
	cout << "*sp2 " << *sp2 << endl;
	*sp2 = 20;
	cout << "*sp " << *sp << endl;
	cout << "*sp2 " << *sp2 << endl;

	return sp;
}

int main()
{
	cout << "---------- 01-sptr.cpp --------------" << endl;
	SmartPtr<int> sp;
	sp = test();
	*sp = 30;
	cout << *sp	<< endl;

	return 0;
}
