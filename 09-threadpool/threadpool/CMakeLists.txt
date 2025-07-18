cmake_minimum_required(VERSION 3.22)

project(threadpool)

option(BUILD_SAMPLES "Is build samples?" ON)
option(BUILD_TESTS "Is build tests?" ON)
option(gtest_build_samples "Is build gtest samples?" ON)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE(GLOB SRCS ${CMAKE_CURRENT_LIST_DIR}/src/*.cc)
FILE(GLOB H_FILE ${CMAKE_CURRENT_LIST_DIR}/include/*.h)

add_library(threadpool SHARED ${SRCS} ${H_FILE})
target_include_directories(threadpool PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_compile_features(threadpool PUBLIC cxx_std_17)

set_target_properties(${PROJECT_NAME}
   PROPERTIES 
   ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out/lib
   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out/lib
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out/bin
)

if (BUILD_SAMPLES)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/samples/)
    add_dependencies(test-threadpool threadpool)
endif()

if (BUILD_TESTS)
    set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/tools/gtest/lib/cmake/)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tools)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/unit_test/)
    add_dependencies(gtest test-threadpool)
    add_dependencies(unit_test gtest)
endif()

