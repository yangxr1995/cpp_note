#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <thread>
#include <utility>

#include "threadpool.h"

const size_t MAX_THREAD_NB = 10;
const size_t MAX_TASK_NB = 1024;

ThreadPool::ThreadPool()
    : initThreadNumber_(0), maxThreadNumber_(MAX_THREAD_NB), taskQueNumber_(0),
    taskQueMaxNumber_(MAX_TASK_NB), mode_(ThreadPoolMode::MODE_FIXED),
    isExit_(false) {

}

ThreadPool::~ThreadPool() {
    // 通知所有线程退出
    isExit_ = true;
    notEmptyOrExit_.notify_all();

    // 等待所有线程退出
    std::cout << "wait for threads_" << std::endl;
    for (auto &pos : threads_) {
        pos->join();
    }

}

void ThreadPool::setMode(ThreadPoolMode mode) { mode_ = mode; }

void ThreadPool::start(size_t initThreadNumber) {
    initThreadNumber_ = initThreadNumber;

    for (int i = 0; i < initThreadNumber_; ++i) {
        auto th = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this));
        threads_.emplace_back(std::move(th));
    }

    for (auto &th : threads_) {
        std::cout << "thread start" << std::endl;
        th->start();
    }
}

void ThreadPool::setTaskQueMaxNumber(size_t nb) { taskQueMaxNumber_ = nb; }

// 提交任务到任务队列,
// 可能失败,
// 若队列满则等待有空闲位置（待处理任务数大于 taskQueMaxNumber_）
// 若1s后没有空闲位置，则返回失败
// Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
std::shared_ptr<Result> ThreadPool::submitTask(std::shared_ptr<Task> sp) {

    // 获得临界区
    std::unique_lock<std::mutex> lock(taskQueMtx_);

    // 确保队列未满，否则等待空位.
    bool ok = notFull_.wait_for(lock, std::chrono::seconds(1), [&] () -> bool {
            return taskQue_.size() < taskQueMaxNumber_;
            });
    if (!ok) {
        auto result = std::make_shared<Result>(sp, false);
        sp->result_ = result;
        return result;
    }

    // 添加任务
    taskQue_.emplace(sp);
    taskQueNumber_++;

    // 通知等待任务的线程有新任务了
    notEmptyOrExit_.notify_one();

    auto result = std::make_shared<Result>(sp, true);
    sp->result_ = result;
    return result;
}

// 工作线程循环执行任务
void ThreadPool::ThreadFunc() {
    for (;;) {
        std::shared_ptr<Task> task;

        { // 临界区工作
            // 获得临界区锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);

            // 确保有待执行任务
            notEmptyOrExit_.wait(lock, [&] () -> bool {
                    return (taskQue_.size() > 0 || isExit_);
                    });

            // 处理退出通知
            if (isExit_) {
                std::cout << "thread exit" << std::endl;
                break;
            }

            // 处理任务通知
            task = taskQue_.front();
            taskQue_.pop();
            taskQueNumber_--;
        }

        // 通知任务队列有空位
        // 因为有资源计数所以本质上是水平触发事件
        // 所以不用使用 notify_all
        notFull_.notify_one();

        // 执行任务
        Any val = task->run();

        // 设置会返回值
        auto result = task->result_.lock();
        if (result != nullptr) {
            result->set(std::move(val));
        }
        else {
            std::cerr << __PRETTY_FUNCTION__ << ": result is nullptr" << std::endl;
        }
    }
}

// 创建工作线程
void Thread::start() {
    th_ = std::make_unique<std::thread>(func_);

    // detach 不靠谱，用join
    // std::thread th(func_);
    // th.detach(); // 子线程虽然和主线程游离，只是意味着子线程退出后的资源会由内核回收，但子线程若死循环，则会导致主线程退出后，进程继续运行
}

// 构造线程类，绑定线程工作函数
Thread::Thread(ThreadFunc func) : func_(func) {}

Thread::~Thread() {}

Result::Result(std::shared_ptr<Task> task, bool isVaild)
    : task_(task), isVaild_(isVaild), sem_(0) {
        // task_->result_ = this;
        // task->result_ = std::shared_ptr<Result>(this);
    }

