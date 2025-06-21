#include <algorithm>
#include <execution>
#include <future>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

int main (int argc, char *argv[]) {
    vector<string> s = {"111", "222", "333"};
    for_each(execution::par, 
            s.begin(), s.end(), 
            [](string &s) {
            cout << this_thread::get_id() <<" " ;
            cout << s << endl;
            });
   
    return 0;
}
