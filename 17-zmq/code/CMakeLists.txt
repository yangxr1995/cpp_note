cmake_minimum_required(VERSION 3.10)
project(zmq-test)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdPartLibs)
set(THIRD_PARTY_INSTALL_DIR ${THIRD_PARTY_DIR}/_install)

function(check_and_add_external_project proj_name lib_files)
    set(needs_build FALSE PARENT_SCOPE)
    foreach(lib_file IN LISTS lib_files)
        if(NOT EXISTS ${THIRD_PARTY_INSTALL_DIR}/lib/${lib_file})
            set(needs_build TRUE PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

include(ExternalProject)

set(needs_build FALSE)
check_and_add_external_project(libzmq "libzmq.a")
ExternalProject_Add(libzmq
    PREFIX ${THIRD_PARTY_DIR}/libzmq
    GIT_REPOSITORY "https://github.com/zeromq/libzmq.git"
    GIT_TAG "v4.3.2"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}
    BUILD_BYPRODUCTS ${THIRD_PARTY_INSTALL_DIR}/lib/libzmq.a
    UPDATE_DISCONNECTED ON
    BUILD_ALWAYS ${needs_build}
)

function(add_exec name srcs libs)
    add_executable(${name} ${srcs})
    target_include_directories(${name}
        PRIVATE ${THIRD_PARTY_INSTALL_DIR}/include)
    target_link_directories(${name}
        PRIVATE ${THIRD_PARTY_INSTALL_DIR}/lib)
    target_link_libraries(${name} ${libs})
endfunction()

add_exec(hwserver hwserver.c zmq)
add_exec(hwclient hwclient.c zmq)

