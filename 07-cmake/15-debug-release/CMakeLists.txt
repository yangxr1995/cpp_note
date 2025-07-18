cmake_minimum_required(VERSION 3.22)
project(debug-release)

# 创建静态库源文件
file(WRITE src/slib.cpp [=[
    void Slib() {
    }
    ]=])

# CMAKE_BUILD_TYPE : 控制编译优化等级和debug参数的变量
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 用户可以在配置时期设置 CMAKE_BUILD_TYPE
# cmake -S . -B build  -D CMAKE_BUILD_TYPE=xxx
# 设置不同值时，cmake时机采用的编译参数:
#     Debug: 
#          -g
#     Release:
#          -O3 -DNDEBUG 
#     RelWithDebInfo:
#          -O2 -g -DNDEBUG
#     MinSizeRel:
#          -Os -DNDEBUG

add_library(slib STATIC src/slib.cpp)

set(LIB_OUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(EXE_OUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# 设置不同类别的静态库slib的输出路径
set_target_properties(slib PROPERTIES 
    # 设置静态库的输出路径
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release
)

file(WRITE include/dlib.h [=[
    void dlib();
    ]=])
file(WRITE src/dlib.cpp [=[
    #include "dlib.h"
    void dlib() {}
    ]=])


add_library(dlib SHARED src/dlib.cpp include/dlib.h)
target_include_directories(dlib PUBLIC include)
set_target_properties(dlib PROPERTIES
    # 设置静态库的输出路径
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release

    # win : 设置dll的输出路径
    RUNTIME_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release

    # linux/mac : 设置.so的输出路径
    LIBRARY_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release
)

file(WRITE src/main.cpp [=[
    #include "dlib.h"
    void Slib();
    int main() {
    Slib();
    dlib();
    }
    ]=])

add_executable(main src/main.cpp)

# 指定bin文件的输出路径
target_link_libraries(main slib dlib)
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${EXE_OUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXE_OUT_PATH}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXE_OUT_PATH}/release)
