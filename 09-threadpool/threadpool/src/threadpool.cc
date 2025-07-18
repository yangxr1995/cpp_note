#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <thread>

#include "threadpool.h"

const size_t MAX_THREAD_NB = 10;
const size_t MAX_TASK_NB = 1024;

ThreadPool::ThreadPool()
    : initThreadNumber_(0), maxThreadNumber_(MAX_THREAD_NB), taskQueNumber_(0),
    taskQueMaxNumber_(MAX_TASK_NB), mode_(ThreadPoolMode::MODE_FIXED) {}

    ThreadPool::~ThreadPool() {}

    void ThreadPool::setMode(ThreadPoolMode mode) { mode_ = mode; }

    void ThreadPool::start(size_t initThreadNumber) {
        initThreadNumber_ = initThreadNumber;

        for (int i = 0; i < initThreadNumber_; ++i) {
            auto th = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this));
            threads_.emplace_back(std::move(th));
        }

        for (auto &th : threads_) {
            th->start();
        }
    }

void ThreadPool::setTaskQueMaxNumber(size_t nb) { taskQueMaxNumber_ = nb; }

// 提交任务到任务队列,
// 可能失败,
// 若队列满则等待有空闲位置（待处理任务数大于 taskQueMaxNumber_）
// 若1s后没有空闲位置，则返回失败
Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {

    // 获得临界区
    std::unique_lock<std::mutex> lock(taskQueMtx_);

    // 确保队列未满，否则等待空位.
    bool ok = notFull_.wait_for(lock, std::chrono::seconds(1), [&] () -> bool {
            return taskQue_.size() < taskQueMaxNumber_;
            });
    if (!ok)
        return Result(sp, false);

    // 添加任务
    taskQue_.emplace(sp);
    taskQueNumber_++;

    // 通知等待任务的线程有新任务了
    notEmpty_.notify_one();

    return Result(sp, true);
}

// 工作线程循环执行任务
void ThreadPool::ThreadFunc() {
    for (;;) {
        // 获得临界区锁
        std::unique_lock<std::mutex> lock(taskQueMtx_);

        // 确保有待执行任务
        notEmpty_.wait(lock, [&] () -> bool {
                return taskQue_.size() > 0;
                });

        // 获得任务
        auto task = taskQue_.front();
        taskQue_.pop();
        taskQueNumber_--;

        // 释放临界区
        lock.unlock();

        // 通知有新空位
        notFull_.notify_one();

        // 执行任务
        Any val = task->run();

        // 设置会返回值
        task->result_->set(std::move(val));
    }
}


// 创建工作线程
void Thread::start() {
    std::thread th(func_);
    th.detach();
}

// 构造线程类，绑定线程工作函数
Thread::Thread(ThreadFunc func) : func_(func) {}

Thread::~Thread() {}

Result::Result(std::shared_ptr<Task> task, bool isVaild)
: task_(task), isVaild_(isVaild), sem_(0) {
    task_->result_ = this;
}

