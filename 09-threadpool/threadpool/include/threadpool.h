#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sys/types.h>
#include <thread>
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
// 信号量是线程通信机制
// 单向传递信息，水平触发
// 信息内容简单，为资源计数
class Semaphore {
    public:
        // 初始资源数量为0
        Semaphore(uint32_t n = 0)
        : n_(n) {}
        ~Semaphore() = default;

        // cv_ 和 mtx_ 不支持拷贝构造和赋值，所以 Semaphore 也无法实现拷贝构造和赋值
        // 为避免编译时错误，显示声明 Semaphore 的拷贝构造和赋值被删除
        //
        // 若一定要让 Semaphore 支持拷贝构造和赋值，
        // 必须修改 cv_ 和 mtx_ 为 指针，
        // 如
        // std::unique_ptr<std::condition_variable> cv_
        // std::unique_ptr<std::mutex> mtx_
        // 绕过 Semaphore 成员不支持拷贝构造和赋值的限制
        Semaphore(const Semaphore &) = delete;
        Semaphore &operator=(const Semaphore &) = delete;

        // 和上述同样的原因，Semaphore也无法实现右值拷贝和右值赋值
        // 若不显示删除 Semaphore的右值拷贝和赋值，
        // Semaphore的右值拷贝和赋值也会被 C++隐式删除
        // 因为只要定义了 构造/析构/拷贝，C++就会隐式删除右值拷贝和右值赋值
        Semaphore(Semaphore &&) = delete;
        Semaphore &operator=(Semaphore &&) = delete;

        // 资源计数加一，并广播通知其他线程
        void post() {
            std::unique_lock<std::mutex> lock(mtx_);
            n_++; // n为临界区资源，所以在锁中
            cv_.notify_all(); // notify_all本身是边沿触发的
                              // 但通过锁和资源计数，
                              // 将边沿触发改为了水平触发
        }

        // 若有资源，则资源计数减一，若无资源则等待
        void wait() {
            // 锁后wait先检查资源的逻辑也是将 cv.notify
            // 将边沿触发改为了水平触发的关键
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [&] () -> bool {
                    return n_ > 0;
                    });
            n_--;
        }

    private:
        std::condition_variable cv_; // 用于线程通信
        std::mutex mtx_; // 锁临界区，将边沿触发改为了水平触发的关键
        uint32_t n_; // 资源计数
};

// Any类型 : 可以封装任意类型，自身是非模板类，所以可以用在虚函数的声明
//
// 实现原理:
// 1. Any的数据为任意类型的基类的指针，目的
//   1.1 基类指针可以指向派生类，并且基类可以转换为派生类。
//   1.2 基类带虚函数，则可以使用 dynamic_cast，利用RTTI确保基类转换为派生类的正确
//   1.3 void * 虽然可以实现指针指向任意类型，但是无法使用RTTI确保类型转换的正确
// 2. 被指向的派生类为模板类。
//   2.1 如此确定类型的基类指针就可以指向任意类型
//   2.2 在编译时生成 基类指针指向派生类对象 和 通过基类指针获得派生类对象 的模板函数
class Any {
    public:
        Any() = default;
        ~Any() = default;

        // 成员为 unique_ptr，只支持右值拷贝构造
        Any(const Any &) = delete; 
        Any(Any &&) = default;
        Any &operator=(const Any &) = delete;
        Any &operator=(Any &&) = default;

        // 最终数据存放在堆上，并使用 unique_ptr 维护声明周期
        template<typename T>
        Any(T data) 
        :data_(std::make_unique<Derive<T>>(data))
        {
        }

        // 通过基类指针获得派生类对象
        template<typename T>
        T cast_() {
            // 基类指针转换为派生类指针
            // static_cast 无法识别运行时错误的类型转换
            // dynamic_cast可以识别运行时错误的类型转换
            // 此处需要识别运行时的类型转换是否正确。
            Derive<T> *pd = dynamic_cast<Derive<T> *>(data_.get());
            // Derive<T> *pd = static_cast<Derive<T> *>(data_.get());
            if (pd == nullptr) { // 通过RTTI检查，若基类和T不存在继承关系，则转换失败
                throw std::runtime_error("type cast err");
            }
            return pd->data_; // 返回真实数据的拷贝
        }

    private:
        // 定义被任意类型继承的基类
        class Base {
            public:
                // 因为要用dynamic_cast所以需要虚函数。
                virtual ~Base() = default;
        };

        // 任意类型继承基类Base
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

// 任务返回值类
// 支持异步获取返回值
// 支持任意类型的返回值
class Result {
    public:
        Result(std::shared_ptr<Task> task, bool isVaild);

        // 因为 Semaphore
        // 左值/右值的拷贝和赋值都被禁用
        Result(const Result &) = delete;
        Result &operator=(const Result &) = delete;

        // 设置返回值，并通知获取者线程
        void set(Any val) 
        {
            val_ = std::move(val);
            sem_.post();
        }

        // 获得真实的返回值，若尚未返回则等待通知
        template<typename T>
        T get() {
            sem_.wait(); //  sem初始值为0，所以会确保任务执行完设置了result，才会继续获取result.val_
            return val_.cast_<T>();  // 返回真实的返回值
        }

        // 所属任务是否派发有效，若无效，则任务的结果也无效
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
        std::weak_ptr<Task> task_;
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
        Thread(const Thread &) = delete;
        Thread & operator=(const Thread &) = delete;
        Thread(Thread &&) = default;
        Thread & operator=(Thread &&) = default;

        void start();

        void join() {
            th_->join();
        }

    private:
        ThreadFunc func_;
        std::unique_ptr<std::thread> th_;
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

        // 相关的异步返回值
        std::weak_ptr<Result> result_;
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

        // Result submitTask(std::shared_ptr<Task> sp); // 添加任务
        std::shared_ptr<Result> submitTask(std::shared_ptr<Task> sp); // 添加任务

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
        std::condition_variable notEmptyOrExit_; // 任务队列未空/退出工作

        ThreadPoolMode mode_; // 当前池的工作模式

        std::atomic_bool isExit_; // 所有线程是否退出
};


#endif
