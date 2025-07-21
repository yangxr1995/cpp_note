#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <utility>
#include "threadpool.h"

template<typename Func, typename ...Args>
auto submitTask(Func &&func, Args&&... args) -> std::future<decltype(func(args...))>
{
    using RetType = decltype(func(args...));
    auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<RetType> ret = task->get_future();
    (*task)();

    return ret;
}


int sum(int a, int b) {
    return a + b;
}

int main (int argc, char *argv[]) {

    {
        ThreadPool tp;
        tp.start(1);

        auto func = [] (int b, int e) {
            int sum = 0;
            for (int i = b; i <= e; ++i) {
                sum += i;
            }
            return sum;
        };
        std::future<int> res1 = tp.submitTask(func, 1, 100);
        std::future<int> res2 = tp.submitTask(func, 1, 100);
        std::future<int> res3 = tp.submitTask(func, 1, 1000000);
        std::future<int> res4 = tp.submitTask(func, 1, 1000000);
        std::future<int> res5 = tp.submitTask(func, 1, 1000000);

        std::future<int> res6 = submitTask(func, 1, 1000000);

        // std::cout << res1.get() << std::endl;
        // std::cout << res2.get() << std::endl;
        // std::cout << res3.get() << std::endl;
        // std::cout << res4.get() << std::endl;
        std::cout << res5.get() << std::endl;
        std::cout << res6.get() << std::endl;
    }
    std::cout << "---------------" << std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
