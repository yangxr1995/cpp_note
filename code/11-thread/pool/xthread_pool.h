#ifndef __XTHREAD_POOL_H__
#define __XTHREAD_POOL_H__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Xtask {
    public:
        virtual int Run() = 0;
        std::function<bool ()> is_exit;

        void set_ret(int val) {
            ret_.set_value(val);
        }
        int get_ret() {
            return ret_.get_future().get();
        }
        // 异步获取返回值
        std::promise<int> ret_;
};

class Xthread_pool {
    public:
        /*
         * 设置线程池的属性
         */
        void Init(unsigned int num);

        /*
         * 启动线程池
         */
        void Start();

        /*
         * 返回线程数量
         */
        unsigned int num();

        void AddTask(std::shared_ptr<Xtask>);

        void Stop();

        bool is_exit() { 
            return is_exit_; 
        }

        unsigned int count_running_task() {
            return count_running_task_;
        }

    private:
        void Run();
        std::shared_ptr<Xtask> GetTask();

        unsigned int num_ = 0;
        std::mutex mux_;
        // std::recursive_mutex mux_;
        std::vector<std::shared_ptr<std::thread>> threads_;
        std::queue<std::shared_ptr<Xtask>> tasks_;
        std::condition_variable cv_;
        bool is_exit_ = false;
        std::atomic<unsigned int> count_running_task_ = 0;
};

#endif
