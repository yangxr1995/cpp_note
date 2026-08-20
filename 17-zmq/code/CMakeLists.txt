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
    target_include_directories(${name}
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/)
    target_link_directories(${name}
        PRIVATE ${THIRD_PARTY_INSTALL_DIR}/lib)
    target_link_libraries(${name} ${libs})
endfunction()

add_exec(hello_srv hello_srv.c zmq)
add_exec(hello_cli hello_cli.c zmq)
add_exec(wuclient wuclient.c zmq)
add_exec(wuserver wuserver.c zmq)
add_exec(tasksink tasksink.c zmq)
add_exec(taskvent taskvent.c zmq)
add_exec(taskwork taskwork.c zmq)
add_exec(pub pub.c zmq)
add_exec(sub sub.c zmq)
add_exec(client client.c zmq)
add_exec(server server.c zmq)
add_exec(inproc inproc.c zmq)
add_exec(ipc_server ipc_server.c zmq)
add_exec(ipc_client ipc_client.c zmq)
add_exec(multi multi.c zmq)
add_exec(tcp_client tcp_client.c zmq)
add_exec(sndmore sndmore.c zmq)
add_exec(sndmore_recv sndmore_recv.c zmq)
add_exec(sndmore_thread sndmore_thread.c zmq)
add_exec(async_poll async_poll.c zmq)
add_exec(client_double_send client_double_send.c zmq)

