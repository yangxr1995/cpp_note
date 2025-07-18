cmake_minimum_required(VERSION 3.20)
project(macro-test)

macro(foo)
    message("ARGN : ${ARGN}")
    cmake_parse_arguments(
        # 前缀
        "MY" 

        # 解析方式
        "DEBUG;TRACE" # options
        "BIN;LIB" # 单value参数
        "TARGETS;SRCS" # 多value参数

        # 被解析的参数数组
        ${ARGN} 
    )
    # 解析生成的参数
    message("MY_DEBUG : ${MY_DEBUG}")
    message("MY_TRACE : ${MY_TRACE}")
    message("MY_BIN : ${MY_BIN}")
    message("MY_LIB : ${MY_LIB}")
    message("MY_TARGETS : ${MY_TARGETS}")
    message("MY_SRCS : ${MY_SRCS}")

    message("MY_UNPARSED_ARGUMENTS : ${MY_UNPARSED_ARGUMENTS}")
    message("MY_KEYWORDS_MISSING_VALUES : ${MY_KEYWORDS_MISSING_VALUES}")
endmacro()

# ARGN : 
# MY_DEBUG : FALSE
# MY_TRACE : FALSE
# MY_BIN : 
# MY_LIB : 
# MY_TARGETS : 
# MY_SRCS : 
foo()

# MY_DEBUG : TRUE
# MY_TRACE : TRUE
# MY_BIN : bin
# MY_LIB : lib
# MY_TARGETS : a.out;a.out2
# MY_SRCS : main.cc;test.cc
foo(DEBUG TRACE 
    BIN "bin" LIB "lib" 
    TARGETS "a.out;a.out2" 
    SRCS "main.cc;test.cc")

# MY_UNPARSED_ARGUMENTS : 
# MY_KEYWORDS_MISSING_VALUES : BIN;LIB;TARGETS
foo(DEBUG TRACE 
    BIN "" LIB ""  # 传递参数为空
    TARGETS "" 
    )

# MY_DEBUG : TRUE
# MY_TRACE : TRUE
# MY_BIN : bin
# MY_LIB : lib
# MY_TARGETS : a.out;a.out2
# MY_SRCS : main.cc;test.cc
# MY_UNPARSED_ARGUMENTS : bin2;lib2
# MY_KEYWORDS_MISSING_VALUES : 
foo(DEBUG TRACE 
    BIN "bin;bin2" LIB "lib;lib2"  # 传递错误的参数
    TARGETS "a.out;a.out2" 
    SRCS "main.cc;test.cc")

