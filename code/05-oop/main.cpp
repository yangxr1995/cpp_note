#include <iostream>
using namespace std;

class Base {
	public:
		Base() {}
		~Base() {}
        int _a;  // 4
};

// 16
class Derive: virtual public Base {
    // Base  4
    // vbptr 8
	public:
        Derive() {}
		~Derive() {}
        int _b; // 4
}; 

// 16
class Derive2: virtual public Base {
    // Derive 16
    // vbptr 8
    public:
        Derive2() {}
        int _c; // 4
};

class Derive3: virtual public Derive2, virtual public Derive {
    public:
        Derive3() {}
        int _d;
};

int main()
{
    Derive3 d;
    d._a = 1;
    d._b = 2;
    d._c = 3;
    d._d = 4;


    int a;

    cout << "Derive size : " << sizeof(Derive) << endl;
    cout << "Derive2 size : " << sizeof(Derive2) << endl;
    cout << "Derive3 size : " << sizeof(Derive3) << endl;

    Derive *pb = &d;
    Derive2 *pb2 = &d;
    Base *pb3 = &d;

    cout << pb->_a << endl;
    cout << pb2->_a << endl;
    cout << pb3->_a << endl;

	return 0;
}

