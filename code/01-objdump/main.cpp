#include <iostream>
using namespace std;


int main (int argc, char *argv[]) {

    int a = 0;
    int b = 2;
    int* p = &a;
    const int* const& qq = p;  // 分配临时变量，让qq指向临时变量
    *p = 1;  // 此时p和临时变量指向同个内存空间，所以 *qq : 1
    p = &b;  // 此时p和临时变量指向不同内存空间
    cout << "*p : " << *p << endl;    // *p : 2 
    cout << "*qq : " << *qq << endl;  // *qq : 1
    //
    // int &&a = 20;
    // int b = a;
    // cout << a << " " << b << endl;
    //
    return 0;
}
