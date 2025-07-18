#include <cstdio>
#include <iostream>
#include <memory>
#include <ostream>
#include <thread>

#include "xthread_pool.h"

using namespace std;

class MyTask : public Xtask {
    public:
        int Run() {
            cout << this_thread::get_id() << " MyTask Run" << endl;
            for (int i = 0; i < 4 && !is_exit(); ++i) {
                this_thread::sleep_for(1s);
                cout << ".";
                cout << name << endl;
                flush(cout);
            }
            cout << "MyTask exit" << endl;
            return 100;
        }
        string name;
        ~MyTask() {
            cout << name << " " << __PRETTY_FUNCTION__ << endl;
        }
};

int main (int argc, char *argv[]) {
    Xthread_pool threads;
    threads.Init(10);
    threads.Start();
    cout << "count running task : " << threads.count_running_task() << endl;

    {
        auto task1 = make_shared<MyTask>();
        task1->name = "111";
        threads.AddTask(task1);

        auto task2 = make_shared<MyTask>();
        task2->name = "222";
        threads.AddTask(task2);
        auto ret = task2->get_ret();
        cout << "task2 ret : " << ret << endl;
    }
    this_thread::sleep_for(3s);
    cout << "count running task : " << threads.count_running_task() << endl;
    threads.Stop();
    cout << "count running task : " << threads.count_running_task() << endl;
    getchar();

    return 0;
}
