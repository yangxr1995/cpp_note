#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include "threadpool.h"

using namespace std;

class MyTask : public Task {

    public:
        MyTask(size_t id)
        : id_(id) {

        }

        Any run() override {
            cout << id_ << " work..." << endl;
            this_thread::sleep_for(chrono::seconds(3));
            cout << id_ << " work end" << endl;
            return Any(std::string("hello"));
        }
    private:
        size_t id_;
};

int main (int argc, char *argv[]) {
    {
        try {
            ThreadPool tp;
            tp.setMode(ThreadPoolMode::MODE_CACHED);
            tp.start(10);

            this_thread::sleep_for(chrono::seconds(3));

            for (size_t i = 0; i < 1000; ++i) {
                tp.submitTask(std::make_shared<MyTask>(i));
            }

            std::cout << "----- --------------wait -------------" << std::endl;
            this_thread::sleep_for(chrono::seconds(30));

            for (size_t i = 0; i < 1000; ++i) {
                tp.submitTask(std::make_shared<MyTask>(i));
            }

            this_thread::sleep_for(chrono::seconds(30));
        }
        catch (runtime_error &e) {
            cout << e.what() << endl;
        }

    }
    cout << "-------" << endl;

    return 0;
}
