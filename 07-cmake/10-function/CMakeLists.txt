cmake_minimum_required(VERSION 3.20)
project(function-test)

function(foo arg1 arg2)
    message("ARGC : ${ARGC}")
    message("ARGN : ${ARGN}")
    message("ARGV0 : ${ARGV0}")
    message("ARGV1 : ${ARGV1}")

    if (NOT ARGV0)
        message("return")
        return()
    endif()

    foreach(var IN LISTS ARGN)
        message("- ${var}")
    endforeach()

endfunction()

# ARGC : 5
# ARGN : 1;2;3
# ARGV0 : OFF
# ARGV1 : bb
# return
foo(OFF "bb" "1" "2" "3")

# ARGC : 5
# ARGN : 1;2;3
# ARGV0 : ON
# ARGV1 : bb
# - 1
# - 2
# - 3
foo(ON "bb" "1" "2" "3")


function(fun arg1)
    # 函数内可以访问全局变量，但设置只在本函数内生效，
    # 相当于函数复制了一个变量
    message("var : ${var}") # main set
    set(var "func set") 

    # 函数内的变量默认作用域是函数
    # 如果希望上层也能访问，必须使用PARENT_SCOPE
    set(ret "fun ret" PARENT_SCOPE)
endfunction()

set(var "main set")
fun("")
message("var : ${var}") # main set

# fun函数传递来的变量只能到本文件作用域，
# 父文件无法访问
message("ret : ${ret}") # fun set
# 如果希望上层父文件也能访问此变量，
# 则需要再次调用 set PARENT_SCOPE
set(ret ${ret} PARENT_SCOPE)
