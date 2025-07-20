#include "threadpool.h"
#include "gtest/gtest.h"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace std;

class MyTask : public Task {
    public:

        MyTask(size_t begin, size_t end)
        : begin_(begin), end_(end) {

        }

        Any run() override {
            size_t i;
            size_t sum = 0;
            for (i = begin_; i < end_ ; ++i) {
                sum += i;
            }
            // this_thread::sleep_for(chrono::microseconds(1));
            return Any(sum);
        }
    private:
        size_t begin_;
        size_t end_;
};

// 测试线程分工
TEST(ThreadPoolTest, TaskDispatchFixed) {

    try {
        size_t sum1 = 0, sum2 = 0;
        ThreadPool tp;
        tp.start(10);
        vector<shared_ptr<Result>> results;

        for (size_t i = 0; i < 10000; ++i) {
            results.push_back(tp.submitTask(make_shared<MyTask>(i, i + 100)));
        }

        sum1 = 0;
        for (auto &pos  : results) {
            if ((*pos).isVaild()) {
                sum1 += (*pos).get<size_t>();
            }
        }

        sum2 = 0;
        for (size_t i = 0 ; i < 10000 ; ++i) {
            for (size_t j = i ; j < i + 100; ++j) {
                sum2 += j;
            }
        }

        EXPECT_EQ(sum1, sum2);
    }
    catch (runtime_error &e) {
        cout << e.what() << endl;
    }

    // 线程使用detach，所以主线程退出时等待1s，确保子线程的资源回收完毕
    this_thread::sleep_for(chrono::seconds(1)); 

    cout << "-----------" << endl;
}

// 测试Cached模式一般使用
// TEST(ThreadPoolTest, TaskDispatchCached) {
//
//     {
//         size_t sum1 = 0, sum2 = 0;
//         ThreadPool tp;
//         tp.setMode(ThreadPoolMode::MODE_CACHED);
//         tp.start();
//         vector<shared_ptr<Result>> results;
//
//         for (size_t i = 0; i < 100; ++i) {
//             results.push_back(tp.submitTask(make_shared<MyTask>(i, i + 100)));
//         }
//
//         sum1 = 0;
//         for (auto &pos  : results) {
//             if ((*pos).isVaild()) {
//                 sum1 += (*pos).get<size_t>();
//             }
//         }
//
//         sum2 = 0;
//         for (size_t i = 0 ; i < 100 ; ++i) {
//             for (size_t j = i ; j < i + 100; ++j) {
//                 sum2 += j;
//             }
//         }
//
//         EXPECT_EQ(sum1, sum2);
//     }
//
//     // this_thread::sleep_for(chrono::seconds(3));
//
//     cout << "-----------" << endl;
// }

int main (int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
