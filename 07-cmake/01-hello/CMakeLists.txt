cmake_minimum_required(VERSION 3.20)

project(xlog-test)

# 相当于 -I
include_directories(./libxlog/)

# 相当于 -L
link_directories(./libxlog/)

# 添加lib对象 xlog
# 下面的target_link_libraries要用
add_library(xlog SHARED ./libxlog/xlog.cc ./libxlog/xlog.h)

# 添加exec对象 a.out
# 下面target_link_libraries要用
add_executable(a.out ./xlog/main.cc)

# 指定连接关系
target_link_libraries(a.out xlog)
