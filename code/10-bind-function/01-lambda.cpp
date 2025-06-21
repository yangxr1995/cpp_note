#include <iostream>
using namespace std;

/*
 * 相当于
 * auto func1 = [a, b] (int c) -> int { return a + b + c; };
 */
class Fun {
	public:
		Fun(int aa, int bb) :a(aa), b(bb) {}
		int operator()(int c) const {
			return a + b + c;
		}
	private:
		int a;
		int b;
};

/*
 * 相当于
 * auto func1 = [a, b] (int c) mutable -> int { a+=2; return a + b + c; };
 */
class Fun2 {
	public:
		Fun2(int aa, int bb) :a(aa), b(bb) {}
		int operator()(int c) {
			a += 2; // 此时 this 指向的对象可以修改
			return a + b + c;
		}
	private:
		int a;
		int b;
};

/*
 * 相当于
 * auto func1 = [&a, &b] (int c) -> int { a+=2; return a + b + c; };
 */
class Fun3 {
	public:
		Fun3(int &aa, int &bb) :a(aa), b(bb) {}
		int operator()(int c) {
			a += 2;
			return a + b + c;
		}
	private:
		int &a;
		int &b;
};

void lambda_demo()
{
	cout << "----------------- lambda 示例 ---------------" << endl;
	/*
	 * lambda 用于定义函数对象
	 *
	 * [导入外部变量] (参数列表) -> 返回值 { 函数体 }
	 */
	auto func1 = [] () -> void {
		cout << "Hello World" << endl;
	};

	// 访问函数对象的 operator()
	func1();

	/*
	 * [导入外部变量] 支持多种传值和传引用
	 * [=] 将函数中所有变量传值
	 * [&] 将函数中所有变量传引用
	 * [=, &a] a传引用，其他传值
	 * [a, b] 只传ab值
	 */
	int a = 1;
	int b = 2;
	auto func2 = [=] (int c) mutable -> int {
		a += 1;
		return a + b + c;
	};
	cout << "func2(3) " << func2(3) << endl;

	/*
	 * [导入外部变量] 的本质
	 */
	cout << "a " << a << endl;
	auto func3 = [&a, &b] (int c) -> int {
		a += 1;
		return a + b + c;
	};
	cout << "func3(3) " << func3(3) << endl;
	cout << "a " << a << endl;


}

int main()
{
	lambda_demo();
	return 0;
}
