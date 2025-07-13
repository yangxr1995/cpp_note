cmake_minimum_required(VERSION 3.22)
project(find_package-test)

file(WRITE main.cc [=[
    #include "slib.h"
    int main() {
    slib();
    return 0;
    }
    ]=])

if (NOT version)
    set(version 1.0)
endif()

# cmake 根据 CMAKE_PREFIX_PATH 变量查找slib的Config
# 返回 slib_FOUND 显示是否找到
find_package(slib ${version})
if (slib_FOUND)
    message("slib found success!")
endif()
# Config中定义了slib目录，slib的的属性有 
# INTERFACE_INCLUDE_DIRECTORIES 记录了头文件的路径
get_target_property(inc slib INTERFACE_INCLUDE_DIRECTORIES)
message("inc : ${inc}")

add_executable(main main.cc)

# 由于main依赖slib
# 所以继承 slib的 INTERFACE_INCLUDE_DIRECTORIES
# 所以能找到slib的公共头文件
target_link_libraries(main slib)


