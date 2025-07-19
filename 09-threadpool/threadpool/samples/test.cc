#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include "threadpool.h"

using namespace std;

class MyTask : public Task {

    public:
        Any run() override {
            cout << "work..." << endl;
            this_thread::sleep_for(chrono::seconds(3));
            cout << "work end" << endl;
            return Any(std::string("hello"));
        }
};

int main (int argc, char *argv[]) {
        ThreadPool tp;
        tp.start(10);

        {
            std::shared_ptr<Result> r = tp.submitTask(make_shared<MyTask>());
            if (!r->isVaild()) {
                cout << "Failed to submitTask" << endl;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }

    cout << "-------" << endl;
    // cout << "result : " << r->get<std::string>() << endl;

    return 0;
}
