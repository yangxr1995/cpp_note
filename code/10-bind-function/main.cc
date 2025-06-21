#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <iostream>
using namespace std;

class Lambada1 {
    public:
        Lambada1() {}
        int operator()(int a, int b) const {
            return a + b;
        }
};

class Lambada2 {
    public:
        Lambada2(int &n1, int &n2)
        : _n1(n1), _n2(n2) {}
        void operator() () const {
            int tmp = _n1;
            _n1 = _n2;
            _n2 = tmp;
        }
    private:
        int &_n1;
        int &_n2;
};
 
int main (int argc, char *argv[]) {

    auto a = [] (int a, int b) -> int {
        return a + b;
    };
    auto aa = Lambada1();
    cout << aa(1, 2) << endl;
    cout << a(1, 2) << endl;

    int n1 = 1, n2 = 2;
    auto b = [&n1, &n2] () -> void {
        int tmp;
        tmp = n1;
        n1 = n2;
        n2 = tmp;
    };
    b();
    cout << n1 << " " << n2 << endl;
    auto bb = Lambada2(n1, n2);
    bb();
    cout << n1 << " " << n2 << endl;


    unique_ptr<FILE, function<void (FILE*)>> ptr(fopen("aa", "w"), [](FILE *fp) {fclose(fp);});

    return 0;
}
