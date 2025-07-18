#include "threadpool.h"
#include "gtest/gtest.h"

#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include "threadpool.h"
#include <iostream>
#include <memory>
#include <thread>
#include <string>

// 模拟的Task类
class MockTask : public Task {
public:
    Any run() override {
        return Any(42);
    }
};

// 测试Result类的构造函数
TEST(ResultTest, Constructor) {
    auto task = std::make_shared<MockTask>();
    Result result(task, true);
    EXPECT_EQ(result.isVaild(), true);
}

// 测试Result类的set方法
TEST(ResultTest, SetMethod) {
    auto task = std::make_shared<MockTask>();
    Result result(task, true);
    Any value(100);
    result.set(std::move(value));
    // 简单验证设置后能正常获取值
    EXPECT_NO_THROW(result.get<int>());
}

// 测试Result类的get方法
TEST(ResultTest, GetMethod) {
    auto task = std::make_shared<MockTask>();
    Result result(task, true);
    Any value(200);
    result.set(std::move(value));
    try {
        int val = result.get<int>();
        EXPECT_EQ(val, 200);
    } catch (const std::exception& e) {
        FAIL() << "Exception thrown: " << e.what();
    }
}

// 测试Result类的isVaild方法，有效情况
TEST(ResultTest, IsValidTrue) {
    auto task = std::make_shared<MockTask>();
    Result result(task, true);
    EXPECT_EQ(result.isVaild(), true);
}

// 测试Result类的isVaild方法，无效情况
TEST(ResultTest, IsValidFalse) {
    auto task = std::make_shared<MockTask>();
    Result result(task, false);
    EXPECT_EQ(result.isVaild(), false);
}

void func(Result &result) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    result.set(Any(std::string("hello")));
}

// 测试多线程场景下通过Result异步获得返回值
TEST(ResultTest, GetSetAsync) {
    auto task = std::make_shared<MockTask>();
    Result result(task, true);
    
    std::thread th(func, std::ref(result));
    th.detach();
    std::cout << "wait for get result" << std::endl;
    EXPECT_EQ(result.get<std::string>(), "hello");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
