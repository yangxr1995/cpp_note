cmake_minimum_required(VERSION 3.20)

project(aux-test)

set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories("./include")

aux_source_directory("." SRCS)
aux_source_directory("./lib" SRCS)

message(STATUS "SRCS: ${SRCS}" )

add_executable(a.out ${SRCS})
