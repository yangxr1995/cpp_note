#include <iostream>
using namespace std;

/*
 * 继承中构造和析构顺序
 * 构造：先调用基类的构造函数，再调用组合对象的构造函数(如果有多个对象，顺序为声明顺序)，再调用派生类的构造函数
 * 析构：顺序相反
 */

class Base {
	public:	
		Base() {
			cout << "Base()" << endl;
		}
		~Base() {
			cout << "~Base()" << endl;
		}
		void show() {
			cout << "Base show" << endl;
		}
		void show(int a) {
			cout << "Base show int" << endl;
		}
		int _base;
};

class Derived : public Base {
	public:
		Derived() {
			cout << "Derived()" << endl;
		}
		~Derived() {
			cout << "~Derived()" << endl;
		}
		void show() {
			cout << "Derived show" << endl;
		}
#if 0
		void show(int a) {
			cout << "Derived show int" << endl;
		}
#endif
		int _derived;
};

int main()
{
	Derived d, *pd;
	Base b, *pb;

	// 1. 基类和派生类的转换
	// 继承关系中类型转换，默认只能从派生到基类
	//
	b = d; // 派生类对象转换为基类对象，可以，因为派生类对象有基类对象的成员
	// a = b; // 基类对象转派生类对象，不行，因为基类对象没有派生类对象的成员

	// pd = &b; // 派生类指针指向基类对象,不可以
	pb = &d; // 基类指针指向派生类对象,可以

	// Derived &dd = b; // 基类引用指向派生类对象，不可以
	Base &bb = d; // 基类引用指向派生类对象，可以

	// 2. 重载和隐藏和覆盖
	// 2.1 重载：在同一作用域下，函数名相同，参数列表不同
	b.show();
	b.show(1);

	// 2.2 隐藏：当派生类中有和基类中同名的成员时，派生类会隐藏基类中的同名成员
	// 由于 Derived 定义了show，所以隐藏掉了基类的同名函数，但 Derived 仍然可以调用
	// 基类的函数，但必须通过指定作用域
	d.show();
	d.Base::show(1);

	// 2.3 覆盖
	// 和虚函数有关

	return 0;
}
