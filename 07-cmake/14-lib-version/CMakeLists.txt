cmake_minimum_required(VERSION 3.22)
project(lib-version)

file(WRITE a.cc  [=[
void A() {}
]=])

file(WRITE main.cc  [=[
int main() {
    return 0;
}
]=])

# 带版本的库只在SO文件中有意义
add_library(a SHARED a.cc)
# 设置a库的版本为 2.0.1 
# SOVERSION ： 更细的版本
# NO_SONAME : ON 不生成带版本的库，OFF生成带版本的库
set_target_properties(a PROPERTIES 
    VERSION "2.0.1"
    SOVERSION "3"
    NO_SONAME OFF
)

add_executable(main main.cc)
target_link_libraries(main a)
# 可执行程序也可以设置软件版本
set_target_properties(main PROPERTIES
    VERSION "2.0.1")
