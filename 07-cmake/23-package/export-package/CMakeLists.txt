cmake_minimum_required(VERSION 3.22)
project(package-test)

file(WRITE include/slib.h "void slib();")

file(WRITE slib.cc [=[
    #include <iostream>
    #include "slib.h"
    using namespace std;
    void slib() {
    cout << "slib" << endl;
    }
    ]=])

add_library(slib STATIC slib.cc include/slib.h)

set_target_properties(slib PROPERTIES PUBLIC_HEADER include/slib.h)

# 不能这样写，因为 
# ${CMAKE_CURRENT_SOURCE_DIR}/include 是编译库时有效，
# 而Config文件中应该记录 install_dir/include 
#
# target_include_directories(slib PUBLIC include)

# 所以必须写成下面形式
target_include_directories(slib PUBLIC 
    # build时有效，install时为空
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    # install时有效，build时为空
    $<INSTALL_INTERFACE:include>
)

install(TARGETS slib

    # 生成config信息
    EXPORT slib

    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

# 安装config信息
install(EXPORT slib
    FILE slibConfig.cmake 
    DESTINATION config
)


