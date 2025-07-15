cmake_minimum_required(VERSION 3.22)
project(xcpp_all)

option(BUILD_XLOG "build xlog?" ON)
option(BUILD_XTHREAD_POOL "build xthread_pool?" ON)
option(BUILD_SAMPLES "build samples?" ON)

option(BUILD_XLOG_TEST "build xlog test?" ON)
option(BUILD_XTHREAD_POOL_TEST "build xthread_pool test?" ON)

if (BUILD_XLOG)
    add_subdirectory(./src/xlog)
endif()

if (BUILD_XTHREAD_POOL)
    add_subdirectory(./src/xthread_pool)
endif()

if (BUILD_SAMPLES)
    add_subdirectory(./src/samples)
    add_dependencies(test_xlog xlog)
    add_dependencies(test_xthread_pool xthread_pool)
endif()

