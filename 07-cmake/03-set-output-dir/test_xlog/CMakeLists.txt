cmake_minimum_required(VERSION 3.20)
project(test)
include_directories(../xlog/)
link_directories(../xlog/build)
add_executable(a.out main.cc)
target_link_libraries(a.out xlog)

