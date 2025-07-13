cmake_minimum_required(VERSION 3.22)
project(gtest-test)


set(GTEST_PATH ${CMAKE_SOURCE_DIR}/gtest)

if (NOT EXISTS ${GTEST_PATH})
# 解压
# cd ${PROJECT_BINARY_DIR}
# cmake -E tar xf ${CMAKE_SOURCE_DIR}/release-1.11.0.tar.gz
# PROJECT_BINARY_DIR : 就是 cmake -B 参数指定的目录
# CMAKE_SOURCE_DIR : cmake的顶层目录
    execute_process(COMMAND 
        ${CMAKE_COMMAND} -E 
        tar xf ${CMAKE_SOURCE_DIR}/release-1.11.0.tar.gz
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR})


# 配置
# cmake -S ${PROJECT_BINARY_DIR}/googletest-release-1.11.0 -B ${PROJECT_BINARY_DIR}/googletest-release-1.11.0/build
    set(GTEST_SOURCE ${PROJECT_BINARY_DIR}/googletest-release-1.11.0)
    execute_process(COMMAND
        ${CMAKE_COMMAND} -S ${GTEST_SOURCE} -B ${GTEST_SOURCE}/build
    )
# 编译
    execute_process(COMMAND
        ${CMAKE_COMMAND} --build ${GTEST_SOURCE}/build)

# 安装
    execute_process(COMMAND
        ${CMAKE_COMMAND} --install ${GTEST_SOURCE}/build --prefix=${GTEST_PATH})
endif()


add_executable(main main.cc)
# 设置CMAKE_PREFIX_PATH ，否则找不到GTest的config文件
set(CMAKE_PREFIX_PATH ${GTEST_PATH}/lib/cmake)
find_package(GTest)
target_link_libraries(main GTest::gtest_main)

# 将gtest测试程序main，将他包装成ctest
include(GoogleTest)
gtest_discover_tests(main)

# 启动测试
enable_testing()

