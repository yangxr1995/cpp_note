#include <memory>
#include <thread>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

#include "sgi-allocator.h"

#include <iostream>
#include "sgi-allocator.h"
#include <type_traits>


int main (int argc, char *argv[]) {

    auto func = []()  {
        for (int i = 0; i < 1000; ++i) {
            vector<int, sgi::allocator<int, true, 0>> v;
            // vector<int, std::allocator<int>> v;

            for (int i = 0; i < 100; ++i) {
                v.push_back(i);
            }

            for (auto &item : v) {
                cout << item << " " ;
            }
            cout << endl;

            v.clear();

            for (int i = 0; i < 100; ++i) {
                v.push_back(i);
            }
        
        }
    };

    thread th1(func);
    thread th2(func);
    thread th3(func);
    thread th4(func);

    th1.join();
    th2.join();
    th3.join();
    th4.join();

    return 0;
}
