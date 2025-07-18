cmake_minimum_required(VERSION 3.22)
project(test-threadpool)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/test.cc)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include/)

target_link_libraries(${PROJECT_NAME} threadpool)

target_link_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../out/lib/
    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../out/bin/)

