cmake_minimum_required(VERSION 3.20)
project(cross-compile)

# 创建文件
file(WRITE a.cpp [=[
    #include <iostream>
    void A() {
    std::cout << "A : " << A_VAR << std::endl;
    }
]=])

add_library(A STATIC a.cpp)

# 给目标A添加编译宏定义 -DA_VAR=123
# A和其依赖者都有该宏定义
target_compile_definitions(A PUBLIC A_VAR=123)

# 修改变量 A.INCLUDE_DIRECTORIES  A.INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(A PUBLIC "/A_PUBLIC")
# 修改变量 A.INCLUDE_DIRECTORIES
target_include_directories(A PRIVATE "/A_PRIVATE")
# 修改变量 A.INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(A INTERFACE "/A_INTERFACE")

# 使用cxx17特性
target_compile_features(A PUBLIC cxx_std_17)

# 导入 CMakePrintHelpers ，为了使用 cmake_print_properties
include(CMakePrintHelpers)
# 打印结果
# Properties for TARGET A:
#   A.INCLUDE_DIRECTORIES = "/A_PUBLIC;/A_PRIVATE"
#   A.INTERFACE_INCLUDE_DIRECTORIES = "/A_PUBLIC;/A_INTERFACE"
cmake_print_properties(TARGETS A PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES)

file(WRITE b.cpp [=[
    #include <iostream>
    void A();
    void B() {
        A();
    std::cout << "B : " << A_VAR << std::endl;
    // std::cout << "B : " << STATIC << std::endl; // error
    }
    ]=])

add_library(B STATIC b.cpp)
# B -> A
target_link_libraries(B PRIVATE A)

# STATIC=1的宏定义只有B的依赖才能使用
target_compile_definitions(B INTERFACE STATIC=1)

# 不能打印继承的属性
cmake_print_properties(TARGETS B PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES)

# 查询B的 include相关属性值
# 只能通过cmake原生调试方法打印继承的属性
# Used includes for target B:
#  * /A_PUBLIC
#  * /A_INTERFACE
set(CMAKE_DEBUG_TARGET_PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES)

file(WRITE main.cpp [=[
    #include <iostream>
    void B();
    int main() {
        B();
        std::cout << "main : " << STATIC << std::endl;
        return 0;
    }
    ]=])

add_executable(a.out main.cpp)
# a.out -> B -> A
target_link_libraries(a.out PRIVATE B)


