cmake_minimum_required(VERSION 3.20)
project(test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
add_library(xlog SHARED ./xlog/xlog.h xlog/xlog.cpp)
include_directories(./xlog)
link_directories(./xlog/build/)
add_executable(a.out ./test_xlog/main.cc)
target_link_libraries(a.out xlog)

