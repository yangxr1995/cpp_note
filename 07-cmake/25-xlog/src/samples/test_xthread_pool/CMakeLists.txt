cmake_minimum_required(VERSION 3.22)
project(test_xthread_pool)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/common.cmake)

cpp_execute(${PROJECT_NAME} xthread_pool)
set(XTHREAD_POOL_LIB_DIR 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../xthread_pool/out/lib/")
set(XTHREAD_POOL_INC_DIR 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../xthread_pool/include/")
target_include_directories(${PROJECT_NAME} PRIVATE ${XTHREAD_POOL_INC_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${XTHREAD_POOL_LIB_DIR})
