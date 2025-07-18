#include <iostream>
#include <memory>
#include <string>
#include "threadpool.h"

using namespace std;

class MyTask : public Task {

    public:
        Any run() override {
            return Any(std::string("hello"));
        }
};

int main (int argc, char *argv[]) {
    ThreadPool tp;
    tp.start(2);

    Result r = tp.submitTask(make_shared<MyTask>());
    if (!r.isVaild()) {
        cout << "submitTask failed" << endl;
        return -1;
    }
    cout << "result : " << r.get<std::string>() << endl;

    exit(0);
}
