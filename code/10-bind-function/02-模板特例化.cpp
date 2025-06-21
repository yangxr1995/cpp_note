#include <iostream>
using namespace std;

/*
 * 要使用特例化必须先定义通用模板
 */
template <typename T>
class vector {
	public:
		vector() {
			cout << "vector()" << endl;
		}
};

/*
 * 优先级
 * 完全特例化 > 部分特例化 > 普通模板
 */

// 完全特例化
template <> // 这个必须写，表示这是一个模板特例化
class vector<char *>  //  这里写怎么特例化
{
	public:
		vector() {
			cout << "vector<char *>()" << endl;
		}
};

// 部分特例化
// 匹配所有元素为指针的情况
template<typename T>
class vector<T*> {
	public:	
		vector() {
			cout << "vector<T *>()" << endl;	
		}
};

// 两个参数函数指针部分特例化
template<typename R, typename A1, typename A2>
class vector<R (*)(A1, A2)> {
	public:
		vector() {
			cout << "vector<R (*)(A1, A2)>()" << endl;	
		}
};

// 两个参数的函数部分特例化
template<typename R, typename A1, typename A2>
class vector<R (A1, A2)> {
	public:
		vector() {
			cout << "vector<R (A1, A2)>()" << endl;
		}
};

void test1()
{
	cout << "------------- 模板特例化 -------------" << endl;
	vector<int> v1;
	vector<char *> v2;
	vector<char (*)(int , int)> v3;
	vector<char (int , int)> v4;
}

template<typename T>
void func1(T a) {
	cout << typeid(T).name() << endl;
}

template<typename R, typename A1, typename A2>
void func2(R(*a)(A1, A2)) {
	cout << typeid(R).name() << endl;
	cout << typeid(A1).name() << endl;
	cout << typeid(A2).name() << endl;
}

template<typename R, typename T, typename A1, typename A2>
void func3(R (T::*a)(A1, A2)) 
{
	cout << "func3" << endl;
	cout << typeid(R).name() << endl;
	cout << typeid(T).name() << endl;
	cout << typeid(A1).name() << endl;
	cout << typeid(A2).name() << endl;
}

class Test {
	public:
		void func(int a , char *b) {}
};

void test2()
{
	cout << "------------- 模板实参推演 -------------" << endl;
	double a;
	func1(a);

	void (*b)(int, int) = nullptr;
	func2(b);

	func3(&Test::func);
}

int main()
{
	test1();
	test2();

	return 0;
}
