#include <functional>
#include <map>
#include <string>
#include <iostream>
using namespace std;


void hello2(string str)
{
	cout << str << endl;
}

int sum(int a, int b)
{
	return a + b;
}

void hello()
{
	cout << "hello world" << endl;
}

class Test {
	public:
		void hello(string str) {
			cout << str << endl;
		}
};

void do_query() {cout << "query" << endl;}
void do_book() {cout << "book" << endl;}
void do_borrow() {cout << "borrow" << endl;}
void do_return() {cout << "return" << endl;}

void function_demo() {

	cout << "---------------- function_demo() ----------------" << endl;
	/*
	 * function 将函数封装为函数对象
	 */
	function<void ()> func1;
	func1 = hello;
	func1();

	function<void (string)> func2 = hello2;
	func2("hello world");

	function<int (int, int)> func3 = sum;
	cout << func3(1, 2) << endl;
	
	function<int (int, int)> func4 = [](int a, int b) -> int {return a - b;}; 
	cout << func4(3, 2) << endl;

//	function<void (Test *, string)> func5 = Test::hello;
//	不能使用 Test::hello 因为 Test::hello不是静态函数，必须依赖对象
//	但这时对象并没有创建
//	所以只能使用 &Test::hello 成员函数指针，指针不依赖对象
	function<void (Test *, string)> func5 = &Test::hello;
	Test t;
	func5(&t, "hello world");

	/*
	 * 将函数对象做map的值
	 * 函数对象比函数指针更灵活，函数指针只能接受函数地址
	 * 函数对象可以和cpp很多机制混用
	 */
	map<int, function<void (void)>> bookList;
	bookList.insert({1, do_query});
	bookList.insert({2, do_book});
	bookList.insert({3, do_borrow});
	bookList.insert({4, do_return});

	int key;
	while (1) {
		cin >> key;
		if (key == 0)
			break;
		auto it = bookList.find(key);
		if (it == bookList.end()) {
			cout << "输入无效" << endl;
		}
		else {
			it->second();
		}
	}
}

template<typename T>
class myfunction
{
};

/*
 * 可变参模板
 */
template<typename R, typename ... A>
class myfunction<R (A ...)>
{
	public:
		using PFUNC = R(*)(A ...);
		myfunction(PFUNC f) 
			:_f(f) {}
		void operator()(A ... a) {
			return _f(a ...);
		}
	private:
		PFUNC _f;
};

void function_implement()
{
	cout << "---------------- function_implement() ----------------" << endl;
	myfunction<void (string)>func = hello2;
	func("1111");

}

int main()
{
	cout << "---------------- 03-functional.cpp ----------------" << endl;
	function_demo();
	function_implement();


	return 0;
}
