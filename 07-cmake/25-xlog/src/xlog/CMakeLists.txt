cmake_minimum_required(VERSION 3.22)
project(xlog)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/common.cmake)
cpp_library(${PROJECT_NAME})

set(_XCPP_NAMESPACE_ "namespace xcpp {")
configure_file(${CMAKE_CURRENT_LIST_DIR}/xconfig.h.in 
    ${CMAKE_CURRENT_LIST_DIR}/include/xconfig.h)

message("BUILD_XLOG_TEST : ${BUILD_XLOG_TEST}")

if (BUILD_XLOG_TEST)
    add_subdirectory(./unit_test)
    add_dependencies(xlog-testmain xlog)
endif()

