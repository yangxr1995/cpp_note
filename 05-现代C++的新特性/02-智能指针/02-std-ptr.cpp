#include <iostream>
#include <memory>
using namespace std;

/*
 * 标准库的不带参数智能指针
 */

int main()
{

	/*
	 * 不带引用参数的智能指针
	 */
	cout << "---------- 02-std-ptr.cpp ---------" << endl;
	cout << "---------- 不带引用参数的智能指针 ------------" << endl;
	{
		/*
		 * 当将指针拷贝给另一个指针时，原指针将被置为 nullptr
		 * auto_ptr(auto_ptr& __a) throw() : _M_ptr(__a.release()) { }
		 */
		cout << "--------- auto_ptr ----------" << endl;
		auto_ptr<int> ap(new int);
		auto_ptr<int> ap2(ap);
		//*ap = 10;
		//cout << *ap << endl;
		cout << *ap2 << endl;
		*ap2 = 10;
		//cout << *ap << endl;
		cout << *ap2 << endl;
	}

	{
		cout << "---------- scoped_ptr ----------" << endl;
//		scoped_ptr<int> sp(new int);
//		scoped_ptr<int> sp2(sp);
//
//		*sp = 10;
	}

	{
		/*
		 * 删除了左值引用的构造，只有右值引用的拷贝构造
      	 * unique_ptr(unique_ptr&&) = default;
         * unique_ptr(const unique_ptr&) = delete;
		 *
		 */
		cout << "---------- unique_ptr ----------" << endl;
		unique_ptr<int> up(new int);
		unique_ptr<int> up2(std::move(up));

		*up2 = 10;
		cout << "*up2 = " << *up2 << endl;
	}

	return 0;
}
