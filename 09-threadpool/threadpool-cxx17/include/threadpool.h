#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <cstdint>
#include <future>
#include <list>
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
#include <chrono>

static const size_t MAX_THREAD_NB = 8;      // 最大线程数，包括动态扩充的线程数量
static const size_t MAX_TASK_NB = 3;     // 最大任务数量
static const size_t TEMP_TASK_MAX_TIME = 5; // 扩充的线程最大生存时间

// 线程池支持的模式
enum class ThreadPoolMode {
    MODE_FIXED,   // 固定数量的线程
    MODE_CACHED,  // 线程数量可动态增长
};

// 线程类型
class Thread {
    public:
        using ThreadFunc = std::function<void (size_t)>;

        // 构造线程类，绑定线程工作函数
        Thread(ThreadFunc func, size_t id) 
            : func_(func), id_(id)
        {
        }

        ~Thread() = default;
        Thread(const Thread &) = delete;
        Thread & operator=(const Thread &) = delete;
        Thread(Thread &&) = default;
        Thread & operator=(Thread &&) = default;

        // 创建工作线程
        void start() {
            std::thread th(func_, id_);
            th.detach(); // 子线程虽然和主线程游离，只是意味着子线程退出后的资源会由内核回收，但子线程若死循环，则会导致主线程退出后，进程继续运行
        }

        void join() {
            th_->join();
        }

        uint32_t getId() {
            return id_;
        }

    private:

        ThreadFunc func_;
        std::unique_ptr<std::thread> th_;
        size_t id_;
};

// 线程池
class ThreadPool {
    public:
        ThreadPool() 
        : initThreadNumber_(0), maxThreadNumber_(MAX_THREAD_NB), taskQueNumber_(0),
        taskQueMaxNumber_(MAX_TASK_NB), mode_(ThreadPoolMode::MODE_FIXED),
        isExit_(false), currThreadNumber_(0), busyThreadNumber_(0) ,
        isStart_(false), allocNextThreadId_(0) {

        }

        ~ThreadPool() {
            // 通知所有线程退出
            isExit_ = true;
            notEmptyOrExit_.notify_all();
        }

        ThreadPool(const ThreadPool &) = delete;
        ThreadPool& operator=(const ThreadPool &) = delete;

        // 提供这些参数的访问，是为了方便单元测试设置指标
        size_t initThreadNumber() const { return initThreadNumber_; }
        size_t maxThreadNumber() const { return maxThreadNumber_; }
        size_t busyThreadNumber() const { return busyThreadNumber_; }
        size_t currThreadNumber() const { return currThreadNumber_; }
        size_t taskQueNumber() const { return taskQueNumber_; }
        size_t taskQueMaxNumber() const { return taskQueMaxNumber_; }

        void start(size_t initThreadNumber) { // 启动线程池
            if (isStart_)
                return;

            isStart_ = true;
            initThreadNumber_ = initThreadNumber;
            for (int i = 0 ; i < initThreadNumber_ ; ++i)
                addThread();
        }

        // 提交任务到任务队列,
        // 可能失败,
        // 若队列满则等待有空闲位置（待处理任务数大于 taskQueMaxNumber_）
        // 若1s后没有空闲位置，则返回失败
        template<typename Func, typename ...Args>
        auto submitTask(Func &&func, Args&&... args) -> std::future<decltype(func(args...))> {

            // 用 packaged_task 包装 func 和 args
            using RetType = decltype(func(args...));
            auto task = std::make_shared<std::packaged_task<RetType()>>
                (std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

            if (!isStart_) {
                goto __ret_null__;
            }

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

                    // taskQue_ 的成员为 function<void ()>
                    // 所以要将 std::packaged_task<RetType ()> 类型的task 封装为 function<void ()> 类型
                    taskQue_.emplace(std::make_shared<std::function<void ()>>([task] () {
                                (*task) ();
                                }));

                    std::cout << "submitTask ok" << taskQue_.size() << std::endl;
                    taskQueNumber_++;
                }
                else { // 队列满了，拒绝服务
                    std::cout << "添加任务失败：待执行任务数已满" << std::endl;
                    goto __ret_null__;
                }
            }

            // 一切准备就绪
            // 通知等待任务的线程有新任务了
            notEmptyOrExit_.notify_one();

            // 提交任务成功，返回 packaged_task 的 future，让客户可以异步获得任务返回值
            return task->get_future();

__ret_null__:
            // 提交任务失败，返回RetType的默认构造
            std::packaged_task<RetType()> taskNULL = std::packaged_task<RetType()>([] () -> RetType {
                    return RetType();
                    });
            taskNULL();
            return taskNULL.get_future();
        }

        void setMode(ThreadPoolMode mode)  {  // 设置池工作模式
            if (isStart_)
                return ;
            mode_ = mode; 
        }

        void setTaskQueMaxNumber(size_t sz) { // 设置任务队列最大任务数
            taskQueMaxNumber_ = sz;
        }

    private:
        // 工作线程循环执行任务
        void ThreadFunc(size_t id) {

            auto lastTime = std::chrono::system_clock::now();

            bool isPredicate = false;

            for (;;) {
                std::shared_ptr<std::function<void ()>> task;

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
                    // std::cout << id << " thread exit by isExit_" << std::endl;
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
                // task当前类型为 shared_ptr<function<void ()>>
                // 内部执行 packaged_task<RetType ()>
                // 所以执行任务后会设置 future的值
                (*task)();

                --busyThreadNumber_;

                lastTime = std::chrono::system_clock::now();
            }

        }

        void addThread() {
            // 创建线程
            auto th = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1), allocNextThreadId_++);
            th->start();

            {
                // 添加线程到线程表
                std::lock_guard<std::mutex> lock(threadsMtx_);
                threads_.emplace_back(std::move(th));
                ++currThreadNumber_;
            }
        }

    private:
        std::list<std::unique_ptr<Thread>> threads_; // 支持自动扩充和收缩的线程组
        std::mutex threadsMtx_; // 线程链表的锁
        size_t initThreadNumber_; // 初始线程数量
        size_t maxThreadNumber_;  // 最大线程数量
        std::atomic_uint busyThreadNumber_; // 繁忙的线程数量 
        std::atomic_uint currThreadNumber_; // 当前总线程数量

        // 因为用户传入的任务类型是 任意类型的返回值，和可变参参数。
        // 因为 ThreadPool 不是模板类。所以其属性不能为模板实现，
        // 只能让其方法为模板函数。
        // 所以 taskQue_ 需要存储固定类型，封装用户传入的任意类型的函数和其参数。
        // 参数用bind将变参转换为无参
        // 返回值用 function 将用户传入的函数封装，
        // 从而实现 function<void ()> 封装一切类型函数对象
        std::queue<std::shared_ptr<std::function<void ()>>> taskQue_; // 任务队列
        std::mutex taskQueMtx_;  // 任务队列锁
        std::atomic_uint taskQueNumber_; // 当前任务数量
        size_t taskQueMaxNumber_; // 最大任务数量

        std::condition_variable notFull_;  // 任务队列未满
        std::condition_variable notEmptyOrExit_; // 任务队列未空/退出工作

        std::atomic_bool isExit_; // 用于通知所有线程的信号变量，线程是否退出

        ThreadPoolMode mode_; // 当前池的工作模式

        bool isStart_; // 线程池是否启动，启动后不能设置配置

        size_t allocNextThreadId_; // 创建Thread时分配id
};

#endif
