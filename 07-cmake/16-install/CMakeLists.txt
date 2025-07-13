cmake_minimum_required(VERSION 3.22)
project(install-pro)

#---------------------------------------------------------------------
# 静态库

# 公开头文件
file(WRITE include/slib.h "void slib();")
# 私有头文件
file(WRITE include/slib_pri.h "#define PRI 1")
file(WRITE slib.cc [=[
    #include "slib.h"
    #include "slib_pri.h"
    void slib() {}
    ]=])

add_library(slib STATIC slib.cc include/slib.h include/slib_pri.h)
target_include_directories(slib PUBLIC include)

#---------------------------------------------------------------------
# 动态库

file(WRITE include/dlib.h "void dlib();")
file(WRITE dlib.cc [=[
    #include "dlib.h"
    void dlib() {}
    ]=])
add_library(dlib SHARED dlib.cc include/dlib.h)
target_include_directories(dlib PUBLIC include)

#---------------------------------------------------------------------
# 执行程序
file(WRITE main.cc [=[
    #include "slib.h"
    #include "dlib.h"
    int main() {
    slib();
    dlib();
    return 0;
    }
    ]=])
add_executable(main main.cc)
target_include_directories(main PUBLIC include)
target_link_libraries(main PUBLIC slib)
target_link_libraries(main PUBLIC dlib)

#---------------------------------------------------------------------
# 统一安装所目标
# 以 ${CMAKE_INSTALL_PREFIX} 为当前目录，
# 结合 DESTINATION 指定的目录 获得安装目录
# 比如下面是 
# ${CMAKE_INSTALL_PREFIX}/bin
message("CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX}")
# install(TARGETS slib dlib main DESTINATION bin)

# cmake -S . -B build  -DCMAKE_INSTALL_PREFIX=./install
# 执行安装 cmake --install ./build
# install
# └── bin
#     ├── libdlib.so
#     ├── libslib.a
#     └── main

# 默认CMAKE_INSTALL_PREFIX 是 /usr/local/


#---------------------------------------------------------------------
# 分类安装

# 设置 slib 的私有头文件和共有头文件
set_target_properties(slib PROPERTIES PUBLIC_HEADER include/slib.h)
set_target_properties(slib PROPERTIES PRIVATE_HEADER include/slib_pri.h)

install(TARGETS slib dlib main # 需要安装的目标
    RUNTIME DESTINATION bin # 可执行程序
    ARCHIVE DESTINATION lib # 静态库
    LIBRARY DESTINATION libso # 动态库
    PRIVATE_HEADER DESTINATION inc_pri # 私有头文件
    PUBLIC_HEADER DESTINATION include  # 共有头文件
)

#---------------------------------------------------------------------
# 区分debug和release版本
install(TARGETS slib dlib main
    CONFIGURATIONS Debug    # debug版本 : 
                            # 只有 cmake --install ./xxx --config Debug 
                            # 才会触发这个install
    RUNTIME DESTINATION debug/bin # 可执行程序
    ARCHIVE DESTINATION debug/lib # 静态库
    LIBRARY DESTINATION debug/libso # 动态库
    PRIVATE_HEADER DESTINATION debug/inc_pri # 私有头文件
    PUBLIC_HEADER DESTINATION debug/include  # 共有头文件
)

install(TARGETS slib dlib main
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel    # debug版本
    RUNTIME DESTINATION release/bin # 可执行程序
    ARCHIVE DESTINATION release/lib # 静态库
    LIBRARY DESTINATION release/libso # 动态库
    PRIVATE_HEADER DESTINATION release/inc_pri # 私有头文件
    PUBLIC_HEADER  DESTINATION release/include  # 共有头文件
)


