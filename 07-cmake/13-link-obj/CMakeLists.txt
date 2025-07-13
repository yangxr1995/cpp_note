cmake_minimum_required(VERSION 3.22)
project(link-obj)

file(WRITE a.cpp [=[
    #include <iostream>
    void A() {
    std::cout << "A " << std::endl;
    }
    ]=])

file(WRITE b.cpp [=[
    #include <iostream>
    void B() {
    std::cout << "B " << std::endl;
    }
    ]=])


file(WRITE main.cpp [=[
    #include <iostream>
    int main() {
     A();
     B();
     return 0;
    }
    ]=])

# 生成虚拟对象testab_obj，用于将 a.cpp b.cpp -> a.cpp.o b.cpp.o
add_library(testab_obj OBJECT a.cpp b.cpp)
# 最终生成动态库所以编译时需要加 fPIC
if (NOT MSVC)
    target_compile_options(testab_obj PRIVATE "-fPIC")
endif()

# 错误找不到 testab_obj
# add_library(testab testab_obj)

# add_library SHARED 会自动添加  -fPIC
# 必须用生成表达式将 $<TARGET_OBJECTS:testab_obj> -> a.cpp.o b.cpp.o
add_library(testab SHARED $<TARGET_OBJECTS:testab_obj>)
