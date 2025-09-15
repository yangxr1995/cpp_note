#pragma once 

#include "Thread.h"
#include "noncopyable.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

class EventLoop;

// per thread one loop 模型
// 需要创建子线程，在子线程中创建loop
class EventLoopThread: noncopyable {
    public:
        using ThreadInitCallback = std::function<void(EventLoop *)>;

        EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                const std::string &name = std::string());
        ~EventLoopThread();
        EventLoop *startLoop();

    private:
        void threadFunc();

        Thread thread_;
        EventLoop *loop_;
        bool exiting_;
        std::mutex mutex_;
        std::condition_variable cond_;
        ThreadInitCallback callback_;
};
