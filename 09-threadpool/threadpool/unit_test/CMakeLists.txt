cmake_minimum_required(VERSION 3.22)
project(unit_test)

# add_test(<target-name> <target-source-name>)
function(add_test name source)
    add_executable(${name} 
        ${source} ${CMAKE_CURRENT_LIST_DIR}/../src/threadpool.cc)
    target_link_libraries(${name} GTest::gtest_main)
    set_target_properties(${name}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../out/unit_test/
    )
    target_include_directories(${name}
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include/
    )
endfunction()

# 设置CMAKE_PREFIX_PATH ，否则找不到GTest的config文件
message("CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")
find_package(GTest)

add_test(any Any_unittest.cc)
add_test(result Result_unittest.cc)
add_test(threadpool_t ThreadPool_unittest.cc)

add_custom_target(unit_test)
