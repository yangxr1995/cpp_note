cmake_minimum_required(VERSION 3.10)
project(mymuduo)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -O0")

aux_source_directory(. SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST})

add_executable(test ${SRC_LIST})

