#include <iostream>
#include <memory>
using namespace std;

/*
 * shared_ptr 带引用计数的智能指针，可以访问资源，且支持多线程
 * weak_ptr 不改变引用计数，不能访问资源，但可以转换为shared_ptr，支持多线程
 */
void test() {
	shared_ptr<int> sp(new int);
	shared_ptr<int> sp2(sp);
	*sp = 10;
	cout << "*sp " << *sp << endl;
	cout << "*sp2 " << *sp2 << endl;
	*sp2 = 20;
	cout << "*sp " << *sp << endl;
	cout << "*sp2 " << *sp2 << endl;
}

class A;
class B {
	public:
		/*
		 * weak_ptr只有通过shared_ptr进行赋值或构造
		 * weak_ptr<A>(shared_ptr<A> p)
		 *
		 * 不能通过普通指针构造
		 *
			B(A *p = nullptr) {
				_p = p;
			}
		*/

		/*
			B(shared_ptr<A> &p) {
				_p = p;
			}
		*/
		~B() {
			cout << "~B" << endl;
		}
		void func() {
			cout << "B::good func" << endl;
		}
		//shared_ptr<A> _p;
		weak_ptr<A> _p;
};

class A {
	public:
		~A() {
			cout << "~A" << endl;
		}
		void func() {
			cout << "A::good func" << endl;
			shared_ptr<B> bp = _p.lock();
			if (bp == nullptr) {
				cout << "资源已经被释放" << endl;
			}
			else {
				bp->func();
			}
		}
		// shared_ptr<B> _p; // 会增加引用计数，但是堆上的对象无法自动释放，导致引用计数不为0，导致内存泄漏
		weak_ptr<B> _p;
};



/*
 * shared_ptr导致的循环引用问题
 */
void test_ref_loop() {
	shared_ptr<A> ap(new A);
	shared_ptr<B> bp(new B);
	ap->_p = bp;
	bp->_p = ap;
	ap->func();
}

int main()
{
	test();
	test_ref_loop();
	return 0;
}
