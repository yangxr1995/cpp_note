cmake_minimum_required(VERSION 3.22)
project(xlog-testmain)

include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/common.cmake)

cpp_test(${PROJECT_NAME})

target_link_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../out/lib/)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

target_link_libraries(${PROJECT_NAME} xlog)

enable_testing()

