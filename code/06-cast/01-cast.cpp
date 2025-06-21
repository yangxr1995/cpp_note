#include <iostream>
using namespace std;

class A {
	public:	
		virtual void show() {}
};

class B : public A {
	public:
		void show() { cout << "B::show()" << endl;}
};

class B2 : public A {
	public:
		void show() { cout << "B2::show()" << endl;}
		void show2() { cout << "B2::show2()" << endl;}
};

class C : public B {
	public:
		void show() {}
};

void func(A *pa)
{
	/*
	 * dynamic_cast:
	 *     0. 根据RTTI进行运行时类型转换检查
	 *     1. 如果支持类型转换则返回非nullptr
	 */
	B2 *pb2 = dynamic_cast<B2 *>(pa);
	if (pb2) {
		pb2->show2();
	}
	else {
		pa->show();
	}
}

int main()
{
	{

		const int *piconst = nullptr;
		const int i = 10;
		int *pi;
		double *pd;

		/*
		 * const_cast :
		 *     0. 用于指针或引用去除const
		 *     1. const_cast<目标类型>(被转换的变量)
		 *     2. 被转换的变量必须为指针类型或引用
		 *     3. 原类型和目标类型必须有关联，如int -> double就不行
		 *
		 */
		pi = const_cast<int *>(piconst);
		pi = const_cast<int *>(&i);
		//	pd = const_cast<double *>(piconst); // error


	}

	{
		/*
		 * static_cast
		 *    0. 只提供编译器认为有联系的类型转换
		 *    1. 为编译阶段检查转换是否合法
		 *    2. 如子类转换为基类指针，能转换，但运行时是否合法编译器不保证
		 */
		int *pi;
		unsigned int *pui;
		short *ps;
		double d;
		short s;

		// pi = static_cast<int *>(&s); // error
		// ps = static_cast<short *>(pi); // error 字节大小变化，不可以
		//pi = static_cast<int *>(pui); // error 内存的使用不同

		// 基类 -> 派生类 指针 引用
		C *pc;
		A *pa;

		pc = static_cast<C *>(pa);

		A a;
		A &aa = a;

		C &cc = static_cast<C &>(aa);
	}

	{
		/*
		 * reinterpret_cast:
		 *     0. 类似于c风格的强制类型转换
		 *     1. 编译器不进行严格类型关联检查
		 */
		int *pi;
		unsigned int *pui;
		short *ps;
		double d;
		short s;

		pi = reinterpret_cast<int *>(&s);
		ps = reinterpret_cast<short *>(pi);
		pi = reinterpret_cast<int *>(pui);
	}

	{
		B b;
		B2 b2;

		func(&b);
		func(&b2);
	}


	return 0;
}
