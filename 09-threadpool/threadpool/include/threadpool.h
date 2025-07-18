#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sys/types.h>
#include <utility>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>


// 非windows环境不需要导出导入描述
#ifndef _WIN32

#define XCPP_API 

#else
// windows环境需要导出导入描述

// xlog_EXPORTS 由cmake自动生成
// 如果cmake编译lib，则会生成宏 threadpool_EXPORTS
#ifdef threadpool_EXPORTS
// 编译库需要修饰导出
#define XCPP_API __declspec(dllexport)
#else
// 链接库需要修饰导入
#define XCPP_API __declspec(dllimport)
#endif

#endif

// 信号量
// 信号量是线程同步机制。
// 主要操作为 P V
// V 减少信号量，若信号量为0则睡眠等待
// P 增加信号量，并通知睡眠的线程
// 基础是使用条件变量和互斥锁实现
class Semaphore {
    public:
        Semaphore(uint32_t n = 0)
        : n_(n) {}
        ~Semaphore() = default;
        Semaphore(const Semaphore &) = delete;
        Semaphore &operator=(const Semaphore &) = delete;

        // 资源计数加一，并广播通知其他线程
        void post() {
            std::unique_lock<std::mutex> lock(mtx_);
            n_++;
            cv_.notify_all();
        }

        // 若有资源，则资源计数减一，若无资源则等待
        void wait() {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [&] () -> bool {
                    return n_ > 0;
                    });
            n_--;
        }
    private:
        std::condition_variable cv_;
        std::mutex mtx_;
        uint32_t n_;
};

// Any类型
// Any类型可以指向任意类型
// 实现原理:
// 1. 基类指针可以指向派生类
// 2. 派生类使用模板实现，则派生类为任意类型的封装
// 于是可以实现指向任意类型的Any类型
// 而Any类型本身不是模板，是确定的类型
class Any {
    public:
        Any() = default;
        ~Any() = default;
        Any(const Any &) = delete;
        Any(Any &&) = default;
        Any &operator=(const Any &) = delete;
        Any &operator=(Any &&) = default;

        template<typename T>
        Any(T data) 
        :data_(std::make_unique<Derive<T>>(data))
        {
        }

        // 将指针转换为对应类型，并返回指向的值
        template<typename T>
        T cast_() {
            // static_cast 无法识别运行时错误的类型转换
            // dynamic_cast可以识别运行时错误的类型转换
            // 此处需要识别运行时的类型转换是否正确。
            Derive<T> *pd = dynamic_cast<Derive<T> *>(data_.get());
            // Derive<T> *pd = static_cast<Derive<T> *>(data_.get());
            if (pd == nullptr) {
                throw std::runtime_error("type cast err");
            }
            return pd->data_;
        }

    private:
        class Base {
            public:
                // 因为要用dynamic_cast所以需要虚函数。
                virtual ~Base() = default;
        };

        template<typename T>
        class Derive : public Base {
            public:
                Derive<T>(T data)
                : data_(data){
                }

                T data_;
        };

        std::unique_ptr<Base> data_;
};

class Task;

// 任务返回类
// 支持异步获取返回值
// 支持任意类型的返回值
class Result {
    public:
        Result(std::shared_ptr<Task> task, bool isVaild);

        // Result(std::shared_ptr<Task> task, bool isVaild)
        // : task_(task), isVaild_(isVaild), sem_(0) {
        // }
        
        // 设置返回值，并通知获取者线程
        void set(Any val) 
        {
            val_ = std::move(val);
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            sem_.post();
        }

        // 获得真实的返回值，若尚未返回则等待通知
        template<typename T>
        T get() {
            sem_.wait(); //  sem初始值为0，一定会等待任务执行完设置了result，才会继续获取result.val_
            return val_.cast_<T>();  // 返回真实的返回值
        }

        bool isVaild() {
            return isVaild_;
        }

    private:
        // 用于返回值获取者和设置者之间的同步
        Semaphore sem_;
        // 存储任意类型的数据
        Any val_;
        // 任务若未下发成功，则返回值无效
        std::atomic_bool isVaild_;
        // 所属的任务
        std::shared_ptr<Task> task_;
};

// 线程池支持的模式
enum class XCPP_API ThreadPoolMode {
    MODE_FIXED,   // 固定数量的线程
    MODE_CACHED,  // 线程数量可动态增长
};

// 线程类型
class XCPP_API Thread {
    public:
        using ThreadFunc = std::function<void ()>;

        Thread(ThreadFunc func);
        ~Thread();
        void start();
    private:
        ThreadFunc func_;
};

// 任务类型基类
class XCPP_API Task {
    public:
        // 任务的工作内容，纯虚函数，用户派生实现具体工作内容
        // 返回任意值Any可以封装任意类型的数据
        virtual Any run() = 0;
        // 模板和虚函数不能结合使用, 因为编译器无法实现。
        // 因为模板会导致对同意义函数，编译器生成多种函数实列。
        // 而虚函数则要求同意义的函数示例只有一份。
        // template<typename T>
        // virtual T run2() = 0;

        Result *result_ = nullptr;
    private:
};

// 线程池
// example :
// class MyTask {
//      public:
//          void run() {}
// };
//
// ThreadPool tp;
// tp.start();
//
// tp.submitTask(std::make_shared<MyTask>());
class XCPP_API ThreadPool {
    public:
        ThreadPool();
        ~ThreadPool();

        void setMode(ThreadPoolMode mode); // 设置池工作模式

        void start(size_t initThreadNumber = 4); // 启动池

        void setTaskQueMaxNumber(size_t sz); // 设置任务队列最大任务数

        Result submitTask(std::shared_ptr<Task> sp); // 添加任务

        ThreadPool(const ThreadPool &) = delete;
        ThreadPool& operator=(const ThreadPool &) = delete;

    private:
        void ThreadFunc(); // 所属线程的工作函数

    private:
        std::vector<std::unique_ptr<Thread>> threads_; // 支持动态增长线程组
        size_t initThreadNumber_; // 初始线程数量
        size_t maxThreadNumber_;  // 最大线程数量

        std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
        std::atomic_uint taskQueNumber_; // 任务数量
        size_t taskQueMaxNumber_; // 任务数量
        std::mutex taskQueMtx_;  // 任务队列锁
        std::condition_variable notFull_;  // 任务队列未满
        std::condition_variable notEmpty_; // 任务队列未空

        ThreadPoolMode mode_; // 当前池的工作模式
};


#endif
