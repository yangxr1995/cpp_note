#include <cstdio>
#include <iostream>
#include <memory>

using namespace std;

/*
 * unique_ptr 和 shared_ptr 的 deleter 默认都是 delete ptr;
 * 如果使用 new [] 或者 fopen 等，则需要自定义 删除函数对象
 *
 * 1. 可见定义unique_ptr 模板时可以传入自己的 deleter
  template <typename _Tp, typename _Dp = default_delete<_Tp>>
	class unique_ptr {
		...
	}
 *
 * 2. deleter 是模板函数对象
 *  template<typename _Tp>
    struct default_delete
    {
      void
      operator()(_Tp* __ptr) const
      {
	delete __ptr;
      }
    };
 */

template<typename Ty>
struct ArrDeleter {
	void operator()(Ty *ptr) const {
		cout << "ArrDeleter operator()(Ty *ptr) const" << endl;
		delete [] ptr;
	}
};

template<typename Ty>
struct FILE_deleter {
	void operator()(Ty *ptr) const {
		cout << "FILE_deleter operator()(Ty *ptr) const" << endl;
		fclose(ptr);
	}
};

int main()
{
	cout << "---------- 05-deleter.cpp ---------" << endl;

	unique_ptr<int, ArrDeleter<int>> parr(new int [10]);
	unique_ptr<FILE, FILE_deleter<FILE>> pfile(fopen("test.txt", "w"));

	fprintf(&(*pfile), "aaaa");

	return 0;
}
