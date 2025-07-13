cmake_minimum_required(VERSION 3.20)
project(generator-exp-test)

add_executable(a.out main.cc)

# target_compile_definitions 用于添加 -D 到目标
# $<$<BOOL:ON>:TEST1=123> 就是生成表达式，
# $<BOOL:ON> 为bool表达式，返回true/false
# 外部是条件表达式 $<condition:true_string> : 
#    当condition为真时，返回true_string
target_compile_definitions(a.out PUBLIC "$<$<BOOL:ON>:TEST1=123>")

# target_include_directories(a.out PUBLIC "$<$<NOT:$<BOOL:${LIB}>>:STATIC>")

add_custom_target(gen COMMAND 
    ${CMAKE_COMMAND} -E echo "$<$<NOT:$<BOOL:${LIB}>>:STATIC>")

#et(LIB OFF)
# target_include_directories(a.out PUBLIC
#     "$<STREQUAL:${LIB},OFF>")

# target_include_directories(a.out PUBLIC
#     "$<EQUAL:123,123>")

add_custom_target(gen_config COMMAND
    ${CMAKE_COMMAND} -E echo "$<CONFIG>")

add_custom_target(gen_platform COMMAND
    ${CMAKE_COMMAND} -E echo "$<PLATFORM_ID:Windows,Linux>")

set(LIB ON)
add_custom_target(gen_if COMMAND
    ${CMAKE_COMMAND} -E echo "$<IF:$<BOOL:${LIB}>,ON_STRING,OFF_STRING>")

# build时获得目标a.out的全路径
# /root/cpp_note/07-cmake/11-generation-exp/build/a.out
set(exp "$<TARGET_FILE:a.out>") 

# build时获得目标a.out的名称
# a.out
set(exp "$<TARGET_PROPERTY:a.out,NAME>") 

add_custom_target(gen_target COMMAND
    ${CMAKE_COMMAND} -E echo "${exp}")

