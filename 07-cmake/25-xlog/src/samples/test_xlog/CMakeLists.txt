cmake_minimum_required(VERSION 3.22)
project(test_xlog)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/common.cmake)

cpp_execute(${PROJECT_NAME} xlog)
set(XLOG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../xlog/out/lib/")
set(XLOG_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../xlog/include/")
target_include_directories(${PROJECT_NAME} PRIVATE ${XLOG_INC_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${XLOG_LIB_DIR})


