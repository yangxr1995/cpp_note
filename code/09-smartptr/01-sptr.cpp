#include <iostream>
using namespace std;

/*
 * 实现不带参数的智能指针
 */

template<typename T>
class SmartPtr {
	public:
		SmartPtr(T *p = nullptr)
		:_p(p) {}

		SmartPtr(SmartPtr &&a) {
			_p = a._p;
			a._p = nullptr;
		}

		SmartPtr(const SmartPtr &a) = delete;

		SmartPtr &operator=(SmartPtr &&a) {
			if (_p != nullptr)
				delete _p;
			_p = a._p;
			a._p = nullptr;
			return *this;
		}

		SmartPtr &operator=(const SmartPtr &a) = delete;

		T &operator*() {
			return *_p;
		}

		T *operator->() {
			return _p;	
		}

		~SmartPtr() {
			if (_p != nullptr)
				delete _p;
		}

	private:
		T *_p;
};

class Test {
	public:	
		void test() {
			cout << "test" << endl;
		}
};

SmartPtr<int> test() {

	SmartPtr<int> sp(new int);
	*sp = 10;
	cout << *sp << endl;

	return sp;
}

int main()
{
	cout << "---------- 01-sptr.cpp --------------" << endl;
	{
		SmartPtr<int> sp;
		sp = test();
		*sp = 20;
		cout << *sp	<< endl;

	}

	return 0;
}
