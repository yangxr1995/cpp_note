# 必须指定头文件路径
include_directories(../xlog/)

# 可以不指定库的路径
# link_directories(../xlog/build)

add_executable(test_xlog main.cc)
target_link_libraries(test_xlog xlog)


