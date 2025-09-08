#include <iostream>
#include <memory>
#include <vector>

using namespace std;

template <typename T>
class Allocator {
    public:

        T *allocate(size_t n) {
            return nullptr;
        }

        void deallocate(T *p) {

        }

};

class Test {
    public:
        Test(int) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
        Test(int, int) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
        Test(const Test &) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
        Test(Test &&) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
        ~Test() {
            cout << __PRETTY_FUNCTION__ << endl;
        }
};

int main (int argc, char *argv[]) {
    vector<Test> v;
    v.reserve(100);

    std::allocator<int> p;
    Test t1(1);
    cout << "--------------------------" << endl;
    v.push_back(t1);    // Test::Test(const Test&)
    v.emplace_back(t1); // Test::Test(const Test&)
    cout << "--------------------------" << endl;
    v.push_back(Test(1));     // Test::Test(int)
                              // Test::Test(Test&&)
                              // Test::~Test()

    v.emplace_back(Test(1));  // Test::Test(int)
                              // Test::Test(Test&&)
                              // Test::~Test()
    cout << "--------------------------" << endl;
    v.emplace_back(1);        // Test::Test(int)
    v.emplace_back(1, 2);     // Test::Test(int, int)
    cout << "--------------------------" << endl;




    return 0;
}

