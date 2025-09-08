#include <chrono>
#include <iostream>
#include <thread>
#include <memory>

using namespace std;

void func(weak_ptr<int> p) {
    auto sp = p.lock();
    if (sp == nullptr) {
        cout << "sp == nullptr" << endl;
        return ;
    }
    *sp = 100;
}


int main (int argc, char *argv[]) {

    shared_ptr<int> sp = make_shared<int>(1);
    cout << *sp << endl;
    thread th(func, sp);
    th.detach();

    cout << *sp << endl;
    this_thread::sleep_for(chrono::seconds(1));
    cout << *sp << endl;
    
    return 0;
}


