cmake_minimum_required(VERSION 3.20)
project(test)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(BUILD_SHARED_LIBS ON)
add_subdirectory(xlog)
add_subdirectory(test_xlog)

option(VAR1 "var1" ON)
option(VAR2 "var2" OFF)
set(VAR3 "var3 string" CACHE STRING "doc: var3 string")

message("VAR1 : ${VAR1}")
message("VAR2 : ${VAR2}")
message("VAR3 : ${VAR3}")

# 目标test_xlog 必须已经添加了(add_executable)
set_property(TARGET test_xlog PROPERTY COMPILE_DEFINITIONS "TEST1=\"test1\"")
set_property(TARGET test_xlog APPEND PROPERTY COMPILE_DEFINITIONS "TEST2=\"test2\"")
set_property(TARGET test_xlog APPEND PROPERTY COMPILE_DEFINITIONS "TEST3=\"test3\"")

set_property(TARGET test_xlog PROPERTY TVAR "tvar=11")
