#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <utility>

#include "threadpool.h"

const size_t MAX_THREAD_NB = 8;      // 最大线程数，包括动态扩充的线程数量
const size_t MAX_TASK_NB = 1024;     // 最大任务数量
const size_t TEMP_TASK_MAX_TIME = 5; // 扩充的线程最大生存时间

ThreadPool::ThreadPool()
    : initThreadNumber_(0), maxThreadNumber_(MAX_THREAD_NB), taskQueNumber_(0),
    taskQueMaxNumber_(MAX_TASK_NB), mode_(ThreadPoolMode::MODE_FIXED),
    isExit_(false), currThreadNumber_(0), busyThreadNumber_(0) ,
    isStart_(false) {

}

ThreadPool::~ThreadPool() {
    // 通知所有线程退出
    isExit_ = true;
    notEmptyOrExit_.notify_all();
}

void ThreadPool::addThread() {
    // 创建线程
    auto th = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
    th->start();

    {
        // 添加线程到线程表
        std::lock_guard<std::mutex> lock(threadsMtx_);
        threads_.emplace_back(std::move(th));
        ++currThreadNumber_;
    }

    // std::cout << "addThread : " << "currThreadNumber_[" << currThreadNumber_ << "]" 
    //     << ", busyThreadNumber_[" << busyThreadNumber_ << "]"
    //     << std::endl;
}

void ThreadPool::setMode(ThreadPoolMode mode) { 
    if (isStart_)
        return ;
    mode_ = mode; 
}

void ThreadPool::start(size_t initThreadNumber) {
    if (isStart_)
        return;

    isStart_ = true;
    initThreadNumber_ = initThreadNumber;
    for (int i = 0 ; i < initThreadNumber_ ; ++i)
        addThread();
}

void ThreadPool::setTaskQueMaxNumber(size_t nb) { 
    if (isStart_)
        return;
    taskQueMaxNumber_ = nb; 
}

// 提交任务到任务队列,
// 可能失败,
// 若队列满则等待有空闲位置（待处理任务数大于 taskQueMaxNumber_）
// 若1s后没有空闲位置，则返回失败
// Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
std::shared_ptr<Result> ThreadPool::submitTask(std::shared_ptr<Task> sp) {

    // 绑定异步返回，必须尽早执行，确保工作线程设置返回值时，能获得已绑定的Result
    auto result = std::make_shared<Result>(sp, false);
    sp->result_ = result;

    if (!isStart_)
        return result;

    // 检查是否需要线程扩充
    if (mode_ == ThreadPoolMode::MODE_CACHED) { //  只有Cached模式支持扩容
        if (currThreadNumber_ <= busyThreadNumber_ + taskQueNumber_) { // 若任务不能立即被执行，则视为线程不足
            if (currThreadNumber_ < MAX_THREAD_NB) // 确保不超过最大线程数
                addThread();
            else
                std::cout << "线程扩容失败: 已到达最大线程数" << std::endl;
        }
    }

    // 操作临界区 任务队列, 添加任务
    {
        std::unique_lock<std::mutex> lock(taskQueMtx_);
        // 确保队列未满，否则等待空位1s，若超时未提交任务则返回失败的result.
        bool isNotFull = notFull_.wait_for(lock, std::chrono::seconds(1), [&] () -> bool {
                return taskQue_.size() < taskQueMaxNumber_;
                });
        // 添加任务
        if (isNotFull) {
            taskQue_.emplace(sp);
            taskQueNumber_++;
            result->setVaild(true);
        }
        else { // 队列满了，拒绝服务
            std::cout << "添加任务失败：待执行任务数已满" << std::endl;
            return result;
        }
    }

    // 一切准备就绪
    // 通知等待任务的线程有新任务了
    notEmptyOrExit_.notify_one();

    return result;
}

// 工作线程循环执行任务
void ThreadPool::ThreadFunc(size_t id) {

    // std::cout << "Thread(" << id << ") start" << std::endl;

    auto lastTime = std::chrono::system_clock::now();

    bool isPredicate = false;

    for (;;) {
        std::shared_ptr<Task> task;

        { // 临界区工作
            // 获得临界区锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);

            // 确保有待执行任务
            if (mode_ == ThreadPoolMode::MODE_CACHED) {
                isPredicate = notEmptyOrExit_.wait_for(lock, std::chrono::seconds(2), [&] () -> bool {
                        return (taskQue_.size() > 0 || isExit_);
                        });
            }
            else {
                notEmptyOrExit_.wait(lock, [&] () -> bool {
                        return (taskQue_.size() > 0 || isExit_);
                        });
                isPredicate = true;
            }

            // 获得待执行任务
            if (taskQue_.size() > 0) {
                task = taskQue_.front();
                taskQue_.pop();
                taskQueNumber_--;
            }
        }

        // 处理退出通知
        if (isExit_) {
            std::cout << id << " thread exit by isExit_" << std::endl;
            --currThreadNumber_;
            break;
        }

        // 超时导致wait_for返回, 且没有新任务或退出事件
        if (!isPredicate && mode_ == ThreadPoolMode::MODE_CACHED) {

            // 多余线程若长时间空闲，则删除他
            {
                std::lock_guard<std::mutex> lock(threadsMtx_);
                if (currThreadNumber_ > initThreadNumber_) { 

                    // 获得上次执行任务至今的时间
                    auto curTime = std::chrono::system_clock::now();
                    auto intervalSec = std::chrono::system_clock::to_time_t(curTime) - std::chrono::system_clock::to_time_t(lastTime);

                    // 若长期未执行任务，则删除他
                    if (intervalSec > TEMP_TASK_MAX_TIME) {
                        --currThreadNumber_;
                        threads_.remove_if([id](const std::unique_ptr<Thread> &pos) -> bool {
                                return (*pos).getId() == id;
                                });

                        std::cout << id << " thread exit by intervalSec" << std::endl;
                        break;
                    }

                }
            }

            // timeout，但多余线程未超过最大空闲时间，必须工作
            continue;
        }

        // 通知任务队列有空位
        // 因为有资源计数所以本质上是水平触发事件
        // 所以不用使用 notify_all
        notFull_.notify_one();

        ++busyThreadNumber_;

        // 执行任务
        Any val = task->run();

        --busyThreadNumber_;

        lastTime = std::chrono::system_clock::now();

        // 设置会返回值
        auto result = task->result_.lock();
        if (result != nullptr) {
            result->set(std::move(val));
        }
        else {
            // throw std::runtime_error("result is nullptr");
            std::cerr << __PRETTY_FUNCTION__ << ": result is nullptr" << std::endl;
        }
    }

    // std::cout << "Thread(" << id << ") exit" << std::endl;
    // std::cout << "currThreadNumber_ : " << currThreadNumber_ << std::endl;
    // NOTE : 打印的 currThreadNumber_ 可能不正确，
    // 因为采用 detach 回收线程，主线程若先退出，则可能导致 currThreadNumber_ 值错误
    // 因为 currThreadNumber_ 是在主线程的栈上分配的。
    // 但这个错误不影响业务逻辑
}

// 创建工作线程
void Thread::start() {
    // th_ = std::make_unique<std::thread>(func_, id_);

    // detach 不靠谱，用join
    std::thread th(func_, id_);
    th.detach(); // 子线程虽然和主线程游离，只是意味着子线程退出后的资源会由内核回收，但子线程若死循环，则会导致主线程退出后，进程继续运行
}

// 构造线程类，绑定线程工作函数
Thread::Thread(ThreadFunc func) 
: func_(func), id_(Thread::allocNextId++) 
{
}

Thread::~Thread() {}

size_t Thread::allocNextId = 0;

Result::Result(std::shared_ptr<Task> task, bool isVaild)
    : task_(task), isVaild_(isVaild), sem_(0) {
        // task_->result_ = this;
        // task->result_ = std::shared_ptr<Result>(this);
    }


uint32_t Task::nextAllocId = 0;
