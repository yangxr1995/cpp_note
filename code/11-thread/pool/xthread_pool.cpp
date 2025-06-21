#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include "xthread_pool.h"

using namespace std;

void Xthread_pool::Init(unsigned int num) {
    unique_lock lock(mux_);
    num_ = num;
}

unsigned int Xthread_pool::num() {
    unique_lock lock(mux_);
    return num_;
}

void Xthread_pool::Run() {
    cout << "\n线程运行" << this_thread::get_id() << endl;
    while (!is_exit()) {
        shared_ptr<Xtask> task = GetTask();
        if (is_exit()) break;
        if (!task) continue;
        count_running_task_++;
        try {
            auto ret = task->Run();
            task->set_ret(ret);
        }
        catch (...) {
        }
        count_running_task_--;
        cout << "----" << endl;
    }
    cout << "线程退出" << this_thread::get_id() << endl;
}

void Xthread_pool::AddTask(shared_ptr<Xtask> task) {
    unique_lock lock(mux_);
    task->is_exit = [this] () {return is_exit();};
    tasks_.push(task);
    lock.unlock();
    cv_.notify_one();
}

shared_ptr<Xtask> Xthread_pool::GetTask() {
    unique_lock<mutex> lock(mux_);

    if (tasks_.empty()) {
        cv_.wait(lock);
    }
    if (tasks_.empty()) {
        return nullptr;
    }
    shared_ptr<Xtask> task = tasks_.front();
    tasks_.pop();

    return task;
}

void Xthread_pool::Stop() {
    is_exit_ = true;
    cv_.notify_all();
    for (auto th : threads_) {
        th->join();
    }
}

void Xthread_pool::Start() {
    unique_lock<mutex> lock(mux_);
    if (num_ == 0) {
        cerr << "Xthread_pool err: 线程池未初始化" << endl;
        return;
    }
    if (!threads_.empty()) {
        cerr << "Xthread_pool err: 线程池已经启动" << endl;
        return;
    }
    for (int i = 0; i < num_; ++i) {
        // auto th = new thread(&Xthread_pool::Run, this);
        auto th = make_shared<thread>(&Xthread_pool::Run, this);
        threads_.push_back(th);
    }
}

