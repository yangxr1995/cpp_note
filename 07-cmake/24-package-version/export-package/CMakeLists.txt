cmake_minimum_required(VERSION 3.22)
project(package-test)

# version默认为1.0
# 用户可以通过 -Dversion=xxx 修改
if (NOT version)
    set(version 1.0)
endif()

file(WRITE include/slib.h "void slib();")

# 模板代码
file(WRITE slib.cc.in [=[
    #include <iostream>
    #include "slib.h"
    using namespace std;
    void slib() {
    cout << "slib ${version}" << endl;
    }
    ]=])

# 会将 slib.cc.in 的内容进行替换,
# 这里会将 ${version} 替换为 具体的版本号
# 输出到文件 slib.cc
configure_file(slib.cc.in "${CMAKE_CURRENT_SOURCE_DIR}/slib.cc")

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
    $<INSTALL_INTERFACE:${version}/include>
)

install(TARGETS slib
    # 生成config信息
    EXPORT slib

    RUNTIME DESTINATION ${version}/bin
    ARCHIVE DESTINATION ${version}/lib
    LIBRARY DESTINATION ${version}/lib
    PUBLIC_HEADER DESTINATION ${version}/include
)

# 安装config信息
install(EXPORT slib
    FILE slibConfig.cmake 
    DESTINATION config/slib-${version}/
)

# 注意：在 cmake 配置阶段就会执行，
# 所以一定要设置 CMAKE_INSTALL_PREFIX
#
# 写入版本信息 
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    # 文件路径
    ${CMAKE_INSTALL_PREFIX}/config/slib-${version}/slibConfigVersion.cmake
    # 版本
    VERSION ${version}
    # 版本兼容方式，主版本号兼容
    COMPATIBILITY SameMajorVersion
)

