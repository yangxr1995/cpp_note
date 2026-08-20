cmake_minimum_required(VERSION 3.10)

project(test)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.cc)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/_install/include/)

target_link_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/_install/lib)

target_link_libraries(${PROJECT_NAME} spdlog)
