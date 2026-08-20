#pragma  once

#include "noncopyable.h"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <sys/types.h>
#include <thread>

class Thread : public noncopyable {
public:
    using ThreadFunc  = std::function<void ()>;
    explicit Thread (ThreadFunc, const std::string &name = std::string());

    void start();
    void join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string &name() const { return name_; }
    static int numCreated() { return numCreated_;}
    ~Thread ();

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    ThreadFunc func_;
    pid_t tid_;
    std::string name_;
    static std::atomic_int32_t numCreated_;
};

