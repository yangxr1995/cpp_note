#include <iostream>
using namespace std;

/*
 * 0. 动态绑定和静态绑定
 * 动态绑定：编译阶段发现调用虚函数，则会通过vptr指针找到vtable，并通过偏移量找到函数地址，并调用。
 * 此时，实际调用的地址是在运行阶段才知道，因为vtable是运行时确定，所以称为动态绑定
 *
 * 静态绑定：编译阶段发现调用函数非虚函数，则连接对应的符号，在连接阶段就确定调用的函数。
 *
 * 1. 虚函数表
 * 当一个类有虚函数时，或者他的基类有虚函数时
 * 每个类有自己的虚函数表
 * 虚函数表存放这个类的虚函数地址
 * 对象通过自己的vptr指针找到自己类的虚函数表
 * 在运行时，通过vptr指针和偏移量找到实际调用的虚函数
 *
 * 2. 覆盖
 * 派生类会继承基类的虚函数，所以派生类一定会虚函数表，且内容为基类的虚函数地址.
 * 如果派生类中定义了虚函数（即使没有写virtual，只要函数名返回类型参数列表和基类虚函数相同，则自动为虚函数）,
 * 则会修改自己的虚函数表，覆盖其中基类虚函数的地址。
 *
 * 当一个类包含至少一个虚函数时，编译器会在对象的内存布局中插入一个指向虚函数表的指针（vptr）。
 * 虚函数表是一个存储虚函数地址的数组，每个虚函数对应表中的一个条目
 *
 * 当调用一个虚函数时，程序会查找对象的虚函数表指针，然后从中找到实际要调用的函数。
 * 派生类可以重写基类的虚函数，即使通过基类指针访问对象，也会调用派生类的函数。
 *
 * 3. 哪些函数无法实现动态绑定
 * 动态绑定的依赖: 
 *     vptr指针（构造完成的对象）
 *     vtable（完成初始化的）
 *
 * 所以构造函数即使定义为虚函数，也不能实现多态，因为此时vtable没有初始化完成，
 * 静态函数不能定义为虚函数，因为没有this，所以找不到vptr
 *
 * 4. 虚析构函数
 * 如果使用了多态，则一定要让析构函数成虚析构函数
 *
 * 5. 纯虚函数和抽象类
 * 纯虚函数 virtual void show() = 0;
 * 当类中有纯虚函数时，这个类就是抽象类，不能创建对象，只能定义指针或引用
 *
 * 6. 虚继承
 * 用于解决多继承导致的冗余问题
 * 被虚继承的类称为虚基类 class B : virtual public A {};
 * 子类对象有vbptr指针，指向vbtable，vbtable记录了基类成员在子类对象中的偏移值
 *
 */

class Base {
	public:
		virtual void show() {
			cout << "Base show" << endl;
		}
		void show(int a) {
			cout << "Base show int" << endl;
		}
		virtual ~Base() {
			cout << "~Base" << endl;
		}
		int _base;
};

class Derived : public Base {
	public:
		void show() {
			cout << "Derived show" << endl;
		}
		void show(int a) {
			cout << "Derived show int" << endl;
		}
		virtual ~Derived() {
			cout << "~Derived" << endl;
		}

		int _derived;	
};

int main()
{
	Derived d;
	Base *pb = &d;

	pb->show();
	/*
	 * Base::show()为虚函数，动态绑定，调用哪个类的show，由对象的vptr指针决定
     10840:   ldr r3, [fp, #-24]  ; 获得pb的值
     10844:   ldr r3, [r3]        ; 读d对象的第一个成员 vptr
     10848:   ldr r3, [r3]        ; *vptr 得到 vtable 的起始4Bypte, 也就是虚函数 show的地址
     1084c:   ldr r0, [fp, #-24]  ; 传参pb的值，即d对象的地址，即this指针
     10850:   blx r3              ; 调用show
	 */

	pb->show(1);
	/*
	 * Base::show(int)不是虚函数，静态绑定，一定调用基类的show
	 10854:   mov r1, #1
     10858:   ldr r0, [fp, #-24]  ; 0xffffffe8
     1085c:   bl  10950 <Base::show(int)>
	 */

	Base *pb2 = new Derived;
	delete pb2; // 编译器发现 Base::~Base()为虚函数，所以进行动态绑定 

	return 0;
}

