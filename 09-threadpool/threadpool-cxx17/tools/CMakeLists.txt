cmake_minimum_required(VERSION 3.22)
project(gtest)

set(GTEST_PATH ${CMAKE_CURRENT_LIST_DIR}/gtest)

if (NOT EXISTS ${GTEST_PATH})
# 解压
    execute_process(COMMAND 
        ${CMAKE_COMMAND} -E 
        tar xf ${CMAKE_CURRENT_LIST_DIR}/googletest-1.17.0.tar.gz
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# 配置
    set(GTEST_SOURCE ${PROJECT_BINARY_DIR}/googletest-1.17.0)
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

add_custom_target(gtest)
