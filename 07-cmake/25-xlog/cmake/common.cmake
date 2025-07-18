

set(GTEST_NAME googletest-release-1.12.0)
set(GTEST_TAR ${CMAKE_CURRENT_LIST_DIR}/../tools/${GTEST_NAME}.tar.gz)
set(GTEST_PATH ${CMAKE_CURRENT_LIST_DIR}/../tools/gtest)

set(TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR}/../tools)
set(CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../src)

####################################
## 获得源文件
macro(get_src_include)
    FILE(GLOB SRCS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
    FILE(GLOB H_FILE ${CMAKE_CURRENT_LIST_DIR}/*.h)
    FILE(GLOB H_FILE_I ${CMAKE_CURRENT_LIST_DIR}/include/*.h)
endmacro()


function(gtest_install)
    # 如果没有安装GTest，则安装
    if (NOT EXISTS ${GTEST_PATH})
        # 解压
        # cd ${PROJECT_BINARY_DIR}
        # PROJECT_BINARY_DIR : 就是 cmake -B 参数指定的目录
        # CMAKE_SOURCE_DIR : cmake的顶层目录
        execute_process(COMMAND 
            tar -xf ${GTEST_TAR} -C ${CMAKE_CURRENT_BINARY_DIR}
            )

        # 配置
        set(GTEST_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${GTEST_NAME})
        message("GTEST_SOURCE : ${GTEST_SOURCE}")
        execute_process(COMMAND
            ${CMAKE_COMMAND} -S ${GTEST_SOURCE} -B ${GTEST_SOURCE}/build
        )

        # 编译
        execute_process(COMMAND
            ${CMAKE_COMMAND} --build ${GTEST_SOURCE}/build)

        # 安装
        execute_process(COMMAND
            ${CMAKE_COMMAND} --install ${GTEST_SOURCE}/build --prefix=${GTEST_PATH})
    endif()


endfunction()

####################################
## 编译库 cpp_library(<name>)
function(cpp_library name)
    message("---------------- cpp_library begin ----------------")

    get_src_include()

    option(${name}_SHARED "${name} is shared " OFF)

    if (${name}_SHARED)
        set(TYPE "SHARED")
    else()
        set(TYPE "STATIC")
    endif()

    add_library(${name} ${TYPE} ${SRCS} ${H_FILE} ${H_FILE_I})

    if (NOT version)
        set(version "1.0" CACHE STRING "version of xlog" FORCE)
    endif()

    if (${name}_SHARED)
        target_compile_definitions(${name}
            PUBLIC ${name}_EXPORTS)
    else()
        target_compile_definitions(${name}
            PUBLIC ${name}_STATIC)
    endif()

    target_include_directories(${name}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/>
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>

        PUBLIC $<INSTALL_INTERFACE:${name}-${version}/include>
    )

    target_compile_features(${name} PRIVATE cxx_std_14)

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()

    # 指定文件输出路径
    set(SUPPORT_BUILD_TYPE Debug Release RelWithDebInfo MinSizeRel " ")
    foreach(type  ${SUPPORT_BUILD_TYPE})
        if (type STREQUAL " ")
            set(type "")
        else()
            STRING(TOUPPER _${type} type)
        endif()
        set_target_properties(${name}
            PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY${type} ${CMAKE_CURRENT_LIST_DIR}/out/bin/
        ARCHIVE_OUTPUT_DIRECTORY${type} ${CMAKE_CURRENT_LIST_DIR}/out/lib/
        LIBRARY_OUTPUT_DIRECTORY${type} ${CMAKE_CURRENT_LIST_DIR}/out/lib/
        PDB_OUTPUT_DIRECTORY${type} ${CMAKE_CURRENT_LIST_DIR}/out/bin/
        )
    endforeach()

    set_target_properties(${name}
        PROPERTIES 
        PUBLIC_HEADER "${H_FILE_I}"
    )

    install(TARGETS ${name}
        EXPORT ${name}
        RUNTIME DESTINATION ${name}-${version}/bin
        LIBRARY DESTINATION ${name}-${version}/lib
        ARCHIVE DESTINATION ${name}-${version}/lib
        PUBLIC_HEADER DESTINATION ${name}-${version}/include
    )

    install(EXPORT ${name}
        FILE ${name}Config.cmake 
        DESTINATION config/${name}-${version})

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake
        VERSION ${version}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake 
        DESTINATION config/${name}-${version})

    message("---------------- cpp_library end   ----------------")

endfunction()

####################################
## 编译执行程序 cpp_execute(<name> [lib1] [lib2...])
function(cpp_execute name)
    message("---------------- cpp_execute begin ----------------")

    get_src_include()
    add_executable(${name} ${SRCS} ${H_FILE} ${H_FILE_I})

    if (ARGV1)
        foreach(lib IN  LISTS ARGN)
            message("lib : ${lib}")
            target_link_libraries(${name} ${lib})
        endforeach()
    endif()

    install(TARGETS ${name}
        RUNTIME DESTINATION bin
    )

    message("---------------- cpp_execute end   ----------------")
endfunction()

function(cpp_test name)
    message("---------------- cpp_test ${name} begin ----------------")

    gtest_install()
    get_src_include()

    add_executable(${name} ${SRCS} ${H_FILE} ${H_FILE_I})

    set(CMAKE_PREFIX_PATH ${GTEST_PATH}/lib/cmake)
    find_package(GTest)

    target_link_libraries(${name} GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(${name})

    install(TARGETS ${name}
        RUNTIME DESTINATION bin
    )

    message("---------------- cpp_test ${name} end   ----------------")
endfunction()
