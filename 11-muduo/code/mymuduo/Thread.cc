#include "Thread.h"
#include "CurrentThread.h"
#include <cstdio>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <utility>

std::atomic_int32_t Thread::numCreated_(0);

Thread::Thread (ThreadFunc func, const std::string &name)
    : started_(false)
      , joined_(false)
      , tid_(0)
      , func_(std::move(func))
      , name_(name)
{
    setDefaultName();
}

void Thread::start()
{
    sem_t sem;
    sem_init(&sem, false, 0);
    started_ = true;
    thread_ = std::make_shared<std::thread>([&]() {
            tid_ = CurrentThread::tid();
            sem_post(&sem);
            func_();
            });
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

Thread::~Thread ()
{
    if (started_ && !joined_)
        thread_->detach();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32];
        std::snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

