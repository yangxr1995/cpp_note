cmake_minimum_required(VERSION 3.22)

project(threadpool-test)

option(BUILD_SAMPLES "Is build samples?" ON)
option(BUILD_TESTS "Is build tests?" OFF)
option(gtest_build_samples "Is build gtest samples?" OFF)

# 设置 Debug 模式的默认编译选项
if (NOT MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
endif()

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/samples/test.cc ${CMAKE_CURRENT_LIST_DIR}/include/threadpool.h)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out/bin/)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

#
# if (BUILD_SAMPLES)
#     add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/samples/)
# endif()
#
# if (BUILD_TESTS)
#     set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/tools/gtest/lib/cmake/)
#     add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tools)
#     add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/unit_test/)
#     add_dependencies(gtest test-threadpool)
#     add_dependencies(unit_test gtest)
# endif()

