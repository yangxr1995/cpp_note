cmake_minimum_required(VERSION 3.10)
project(test_gtest)

include(FetchContent)
# 下载googletest
FetchContent_Declare(
    googletest
    URL 
    https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
)

# 编译安装googletest
FetchContent_MakeAvailable(googletest)

#  上述编译安装
#  执行下述指令完成gtest的编译安装
#  1. 下载gtest
#  cmake -S . -B build
#  2. 编译gtest
#  cmake --build build -v
#  3. 安装gtest
#  cmake --install build --prefix=./

# main.cc 单元测试代码
add_executable(main main.cc)
# 设定链接库，头文件自动推导
target_link_libraries(main PUBLIC GTest::gtest_main)

# 将gtest测试程序main，将他包装成ctest
include(GoogleTest)
gtest_discover_tests(main)

# 启动测试
enable_testing()
