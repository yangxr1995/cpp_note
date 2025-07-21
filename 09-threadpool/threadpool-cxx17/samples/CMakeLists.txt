cmake_minimum_required(VERSION 3.22)
project(test-threadpool)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/test.cc ${CMAKE_CURRENT_LIST_DIR}/../include/threadpool.h)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include/)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../out/bin/)

