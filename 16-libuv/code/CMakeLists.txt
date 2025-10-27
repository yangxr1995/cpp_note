cmake_minimum_required(VERSION 3.10)
project(libuv-test)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdPartLibs)
set(THIRD_PARTY_INSTALL_DIR ${THIRD_PARTY_DIR}/_install)

set(needs_build FALSE)
function(check_and_add_external_project proj_name lib_files)
    set(needs_build FALSE PARENT_SCOPE)
    foreach(lib_file IN LISTS lib_files)
        if(NOT EXISTS ${THIRD_PARTY_INSTALL_DIR}/lib/${lib_file})
            set(needs_build TRUE PARENT_SCOPE)
        endif()
    endforeach()
    
    if(needs_build)
        message(STATUS "need build ${proj_name}")
        # 保留原有的ExternalProject_Add调用
    else()
        message(STATUS "${proj_name} already installed")
    endif()
endfunction()

include(ExternalProject)

check_and_add_external_project(libuv "libuv.a")
ExternalProject_Add(libuv
    PREFIX ${THIRD_PARTY_DIR}/libuv
    GIT_REPOSITORY "https://github.com/libuv/libuv.git"
    GIT_TAG "v1.x"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}
    BUILD_BYPRODUCTS ${THIRD_PARTY_INSTALL_DIR}/lib/libuv.a 
    UPDATE_DISCONNECTED ON
    BUILD_ALWAYS ${needs_build}
)


function(add_test name srcs)
    add_executable(test test.cc)
    target_include_directories(test
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_directories(test
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    target_link_libraries(test uv)
endfunction()

add_executable(EvaCli EvaCli.cc)
target_include_directories(EvaCli
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(EvaCli
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(EvaCli uv)

add_executable(EvaSrv EvaSrv.cc)
target_include_directories(EvaSrv
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(EvaSrv
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(EvaSrv uv)


