cmake_minimum_required(VERSION 3.22)
project(samples)

# 获取当面目录下所有子目录
FILE(GLOB dirs LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/*)

set(samps)
foreach(dir IN LISTS dirs )
    if (EXISTS ${dir}/CMakeLists.txt)
        list(APPEND samps ${dir})
    endif()
endforeach()

foreach(samp IN LISTS samps)
    message("samp : ${samp}")
    add_subdirectory(${samp})
endforeach()

message("---------- samples -----------")
