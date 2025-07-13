# 快速入门cmake

## 介绍cmake

- 使用cmake的目的
  - 可用于实现持续集成
  - 跨平台

- cmake的特性
  - 自动搜索可能需要的程序，库和头文件的能力.
  - 独立构建目录，方便清理
  - 支持创建复杂的自定义命名，不依赖shell
  - 配置时选择可选组件的能力

- cmake的工作原理
  - cmake根据CMakeList.txt生成选中环境的构建脚本，使用对应平台的构建工具生成执行程序
  - CMakeCache 和 CMakeLists.txt 可导出变量给 make使用
    - 可以重新生成makefile，仅修改CMakeLists.txt或CMakeCache，就能调整构建过程
                                                                                              
          CMakeCache                                                                           make构建程序
               ▲                                                      ┌───► Makefile ──────────────────────────────────► Linux可执行程序
               │ 2. 生成                                              │
               │                                                      │
               │              3. 根据选中的构建工具，生成对应的文件   │
          cmake执行程序 ──────────────────────────────────────────────┤     
               │                                                      │                       
               │                                                      │                        devenv.exe构建程序
               │ 1. 读取                                              └───► VS项目   ──────────────────────────────────► windows可执行程序
               ▼                                                            
          CMakeLists.txt

## cmake第一个示例
```bash
cmake_minimum_required(VERSION 3.20)

project(xlog-test)

# 相当于 -I
include_directories(./libxlog/)

# 相当于 -L
link_directories(./libxlog/)

# 添加lib对象 xlog
# 下面的target_link_libraries要用
# add_library(xlog SHARED ./libxlog/xlog.cc ./libxlog/xlog.h)
# add_library(xlog STATIC ./libxlog/xlog.cc ./libxlog/xlog.h)

# 添加exec对象 a.out
# 下面target_link_libraries要用
add_executable(a.out ./xlog/main.cc)

# 指定连接关系
target_link_libraries(a.out xlog)
```

# 常用功能
## 注释
- 块注释
```bash
# 行注释
```
- 行注释
```bash
#[[
块注释
]]
```

## message 打印日志

### 默认使用
```bash
message (arg1 arg2 arg3 ..)
```
指定日志级别
查找库日志

```bash
cmake_minimum_required(VERSION 3.20)
project(message-test)

# 行注释

#[[
 块注释
]]

message(参数1)
message(参数1 参数2)
```
控制台输入
```bash
❯ cmake -S . -B build
-- The C compiler identification is GNU 13.3.0
-- The CXX compiler identification is GNU 13.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
参数1
参数1参数2
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /root/cpp_note/07-cmake/02-message/build
```
### 使用日志级别

```bash
message([<mode>] "message text")
```
可以使用的mode
- FATAL_ERROR : 停止cmake运行，输出到stderr
- SEND_ERROR  : cmake继续运行，输出到stderr
- WARNING     : 输出到stderr 
- none or NOTICE : 输出到stderr
- STATUS      : 用户可能感兴趣的信息
- VERBOSE     : 用户可能感兴趣的详细信息
- DEBUG       : 开发者使用的信息
- TRACE       : 开发者使用的跟踪信息


cmake 命令行指定输出的日志级别, 默认为NOTICE
```bash
cmake --log-level=<ERROR|WARNING|NOTICE|STATUS|VERBOSE|DEBUG|TRACE>
```

用于查找库的日志
- CHECK_START : 开始查找某个库
- CHECK_PASS  : 某个库查找结束——找到
- CHECK_FAIL  : 某个库查找结束——未找到

```bash
message(CHECK_START "查找xlog")
message(CHECK_START "查找pthread")
message(CHECK_START "查找rt")
message(CHECK_PASS "Found")
message(CHECK_PASS "Found")
message(CHECK_FAIL "Not found")
message(STATUS "--------")
```
程序输出
```bash
❯ cmake -S . -B build
-- 查找xlog
-- 查找pthread
-- 查找rt
-- 查找rt - Found
-- 查找pthread - Found
-- 查找xlog - Not found
-- --------
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /root/cpp_note/07-cmake/02-message/build
```
## 变量入门
### 变量设置
创建变量，或修改变量
```bash
set(var value)  # 注意var 和 value使用空格隔开，而非逗号
```
删除变量 
```bash
unset(var)
```
变量访问, 若变量没有设置，则返回空字符串。
```bash
${var}
# 变量可以嵌套使用，即var存储变量名
${${var}}
```
示例
```bash
set(VAR1 "aa")
set(VAR2 "VAR1")
message(STATUS "VAR1 = ${VAR1}")     # VAR1 = aa
message(STATUS "VAR2 = ${VAR2}")     # VAR2 = VAR1
message(STATUS "$\{VAR2\} = ${${VAR2}}")  # ${VAR2} = aa
message(STATUS "VAR1 = ${VAR1}")     # VAR1 = 
```

### cmake内建变量
cmake大致有4类目的的内建变量，下面例举几个示例，更多变量需要查询cmake官方首次

- 提供信息的变量
  - PROJECT_NAME
    - project()设置的项目名
- 改变行为的变量
  - BUILD_SHARED_LIBS
    - 缓存变量
    - 通过add_library()设置
    - 可选值：ON(创建共享库) OFF(创建静态库)
- 描述系统的变量
  - UNIX
  - MSVC
- 控制构建过程的变量
  - CMAKE_COLOR_MAKEFILE
    - 控制make是否输出颜色

## include函数
include函数用于cmake文件的分离，
```bash
include(file [OPTIONAL] [RESULT_VARIABLE VAR])
```
从给定的文件读取CMake代码。在该文件中的命令会被立即处理。
如果指定了 OPTIONAL选项，那么如果被包含的文件不存在，不会报错。
如果指定了RESULT_VARIABLE VAR，若成功加载文件，则VAR记录文件的路径，若失败，则VAR内容为NOTFOUND

## 自动查找所有源码文件

```bash
# 将src目录下所有源码文件存入SRCS变量 
aux_source_directory("./src" SRCS)
```

```bash
# 通过通配符自动查找头文件
FILE(GLOB H_FILE1 "${INCLUDE_PATH}/xcpp/*.h")
FILE(GLOB H_FILE2 "${INCLUDE_PATH}/*.h")
```

示例
```bash
# 头文件只需要指定目录，因为会用g++ -M功能找到依赖的具体头文件
include_directories("./include")

# 位于不同目录下的源文件可以写入同个变量
aux_source_directory("." SRCS)
aux_source_directory("./lib" SRCS)

message(STATUS "SRCS: ${SRCS}" )

add_executable(a.out ${SRCS})
```
## cmake分步构建程序
- 查看cmake可选的分步构建
```bash
❯ cmake --build ./build --target help
The following are some of the valid targets for this Makefile:
... all (the default if no target is provided)
... clean
... depend
... edit_cache
... rebuild_cache
... a.out
... lib/xlog.o
... lib/xlog.i
... lib/xlog.s
... main.o
... main.i
... main.s
```

- 预处理
```bash
❯ cmake --build ./build --target main.i
Preprocessing CXX source to CMakeFiles/a.out.dir/main.cc.i

❯ find ./ -name main.cc.i
./build/CMakeFiles/a.out.dir/main.cc.i
```

- 编译
```bash
❯ cmake --build ./build --target main.s
Compiling CXX source to assembly CMakeFiles/a.out.dir/main.cc.s
```
- 汇编
```bash
❯ cmake --build ./build --target main.o
Building CXX object CMakeFiles/a.out.dir/main.cc.o
```
- 链接
```bash
❯ cmake --build ./build --target a.out
[100%] Built target a.out
```

## cmake打印构建具体执行过程
有两个方法:
- 设置变量CMAKE_VERBOSE_MAKEFILE为ON
- 命令行加-v参数
```bash
cmake --build ./build -v
```

## cmake设置输出路径
- CMAKE_LIBRARY_OUTPUT_DIRECTORY
  - 动态库的输出路径
- CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  - 静态库的输出路径
- CMAKE_RUNTIME_OUTPUT_DIRECTORY
  - 程序的输出路径

## add_subdirectory
将项目分成多个CMakeLists.txt清单描述。

主CMakeLists.txt
主CMakeLists.txt定义的变量会传递给子清单
```bash
cmake_minimum_required(VERSION 3.20)
project(test)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(BUILD_SHARED_LIBS ON)
add_subdirectory(xlog)
add_subdirectory(test_xlog)
```

```bash
❯ cat xlog/CMakeLists.txt
add_library(xlog xlog.cpp xlog.h)
```

```bash
❯ cat test_xlog/CMakeLists.txt
# 必须指定头文件路径
include_directories(../xlog/)

# 可以不指定库的路径
# link_directories(../xlog/build)

add_executable(test_xlog main.cc)
target_link_libraries(test_xlog xlog)
```

# cmake主要语法

## 变量
### 普通变量
作用域是当前和子目录，无cache

### 缓存变量
#### 缓存变量和普通变量的区别
- 缓存变量一旦被设置，会存储在cache目录里
  - 类似于数据库中的值，普通变量类似于脚本中的值
- 作用域不同
  - 缓存变量的作用域是全局的。
  - 普通变量的作用域是自身和子目录，不包括上级目录。
- cmake-gui ccmake可配置缓存变量
  - 用户在不修改CMakeLists.txt情况下，通过cmake-gui ccmake可以调整构建。

#### 基础语法
```bash
set(<variable> <value> ... CACHE <type> <docstring> [FORCE])
```
- variable : 缓存变量名
- value ... : 变量值
- CACHE : 表示是缓存变量
- type : 缓存变量的类型，给cmake-gui或ccmake使用
  - BOOL : ON/OFF的选择框
  - FILEPATH : 文件选择
  - PATH ：目录选择
  - STRING : 一行文本
  - INTERNAL : 一行文本，不会开发给用户设置

- docstring : 解释缓存变量的说明信息
- FORCE : 设置此值，缓存变量可以修改，否则一旦创建就不能修改(只能清空，重新创建)

示例
```bash
set(VAR1 "VAR1 init" CACHE STRING "var1 cache var")
message("VAR1 : ${VAR1}") # VAR1 init
set(VAR1 "VAR1 modify" CACHE STRING "var1 cache var")
message("VAR1 : ${VAR1}") # VAR1 init
set(VAR1 "VAR1 modify" CACHE STRING "var1 cache var" FORCE)
message("VAR1 : ${VAR1}") # VAR1 modify 
```

option
快速定义bool类型的缓存变量 
```bash
option(<variable> "<docstring>" <value>)
```

#### 使用ccmake和cmake-gui配置缓存变量
ccmake是linux使用
```bash
# 生成cache目录
cmake -S . -B ./build
# 指向ccmake 指定cache目录
# ccmake中可以修改缓存变量
ccmake ./build
# 确认好缓存变量后，就可以构建程序
cmake --build  ./build --target all
```
#### CACHE覆盖策略
设置缓存变量时，如果已存在同名普通变量，那么应该覆盖普通变量，还是同时构造普通变量和缓存变量？
默认情况是不覆盖。
若不希望覆盖，则需要设置策略CMP0126，
```bash
make_policy(SET CMP0126 NEW)  # 使用新的策略，不会删除同名普通变量 (默认使用新策略)
make_policy(SET CMP0126 OLD)  # 使用旧的策略，会删除同名普通变量
```
设置策略后同时存在普通变量和缓存变量，由于两者同名，所以缓存变量的访问方式有变化。
```bash
$CACHE{VAR} # 访问缓存变量
${VAR} # 访问普通变量 
```

#### 命令行-D传递缓存变量
```bash
# 通过命令行设置或创建缓存变量VAR1
cmake -S . -B build -D VAR1=value1
```


### 环境变量
##### 语法
```bash
# 写
set(ENV{<variable>} [<value>])
# 读
$ENV{<variable>}
```
##### 特性
只影响当前cmake进程, 不影响后续构建和测试进程的环境
类似于全局属性，作用域是全局的，无cache，相对于全局属性，使用更方便
cmake继承了父进程的环境变量，也就是系统环境变量，所以可以在CMakeLists.txt中使用系统环境变量

##### 示例
```bash
# 自定义环境变量（全局作用域）
set(ENV{test} "test1")
message("env test : $ENV{test}") # env test : test1

# 系统环境变量 
message("PATH : $ENV{PATH}")
message("USER : $ENV{USER}")
```
### 属性
- 属性和普通变量的区别
  - 属性是特殊的变量，他是作用域为目标的变量，相当于全局作用域，无cache
  - 普通变量是局部作用域，无cache
  - 缓存变量是全局作用域，有cache

#### set_property
```bash
# 设置属性值
set_property(
              # 可选的目标
             <GLOBAL                      |
              DIRECTORY [<dir>]           |
              TARGET    [<target1> ...]   |
              SOURCE    [<src1> ...]
                        [DIRECTORY <dirs> ...]
                        [TARGET_DIRECTORY <targets> ...] |
              INSTALL   [<file1> ...]     |
              TEST      [<test1> ...]
                        [DIRECTORY <dir>] |
              CACHE     [<entry1> ...]    >

              # APPEND : 以数组方式添加
              # APPEND_STRING :  以字符串拼接方式添加
              [APPEND] [APPEND_STRING]

              # 属性名和属性值
              PROPERTY <name> [<value1> ...])
```
#### get_property
```bash
# 获得属性值或属性文档
get_property(
             # 存放返回值的变量
             <variable>

             # 属性的所属对象
             <GLOBAL             |
              DIRECTORY [<dir>]  |
              TARGET    <target> |
              SOURCE    <source>
                        [DIRECTORY <dir> | TARGET_DIRECTORY <target>] |
              INSTALL   <file>   |
              TEST      <test>
                        [DIRECTORY <dir>] |
              CACHE     <entry>  |
              VARIABLE           >

             # 属性名
             PROPERTY <name>

             # 特殊的读取方式（默认是获得属性值，下面是获得属性的其他信息）
             # SET : 属性是否被设置 (是否调用了 set_property)
             # DEFINED : 属性是否被定义 (是否调用了 define_property)
             # BRIEF_DOCS : 属性的简短描述文档
             # FULL_DOCS : 属性的完整描述文档
             [SET | DEFINED | BRIEF_DOCS | FULL_DOCS])
```

#### define_property
```bash
# 定义属性的说明文档
define_property(
                 # 属性所属对象
                 <GLOBAL | DIRECTORY | TARGET | SOURCE |
                 TEST | VARIABLE | CACHED_VARIABLE>

                 # 属性名称
                 PROPERTY <name> [INHERITED]

                 # 定义属性的信息
                 [BRIEF_DOCS <brief-doc> [docs...]]
                 [FULL_DOCS <full-doc> [docs...]]
                 [INITIALIZE_FROM_VARIABLE <variable>])
```

#### 示例
##### 1. 属性的全局作用域特性
```bash
# main CMakeLists.txt

add_subdirectory(sub)

set_property(GLOBAL PROPERTY main_global "test1")
get_property(main_property_var GLOBAL PROPERTY main_global)
message("main_property_var : ${main_property_var}") # test1
get_property(sub_property_var GLOBAL PROPERTY sub_global)
message("sub_property_var : ${sub_property_var}") # test2
```

```bash
# sub CMakeLists.txt
set_property(GLOBAL PROPERTY sub_global "test2")
```

##### 2. APPEND 和 APPEND_STRING的差别
```bash
set_property(GLOBAL PROPERTY main_global "test1")
set_property(GLOBAL APPEND PROPERTY main_global "test2")
set_property(GLOBAL APPEND PROPERTY main_global "test3")
get_property(main_property_var GLOBAL PROPERTY main_global)
message("main_property_var : ${main_property_var}") # test1;test2;test3
```
```bash
set_property(GLOBAL PROPERTY main_global "test1")
set_property(GLOBAL APPEND_STRING PROPERTY main_global "test2")
set_property(GLOBAL APPEND_STRING PROPERTY main_global "test3")
get_property(main_property_var GLOBAL PROPERTY main_global)
message("main_property_var : ${main_property_var}") # test1test2test3
```

##### 3. define_property
```bash

set_property(GLOBAL PROPERTY main_global "test1")
get_property(property_var_is_defined GLOBAL PROPERTY main_global DEFINED)
message("property_var_is_defined : ${property_var_is_defined}") # 0

# 注意属性只能定义一次,如果有多次定义，只有第一个定义有效 
define_property(GLOBAL PROPERTY main_global FULL_DOCS "test1 is test")
get_property(property_var_is_defined GLOBAL PROPERTY main_global DEFINED)
message("property_var_is_defined : ${property_var_is_defined}") # 1
get_property(property_var_full_docs GLOBAL PROPERTY main_global FULL_DOCS)
message("property_var_full_docs : ${property_var_full_docs}") # test1 is test
```

##### 4. 文件属性
```bash
set_property(SOURCE main.cc PROPERTY test "test1")
get_property(var SOURCE main.cc PROPERTY test)
message("var : ${var}") # test1

# COMPILE_DEFINITIONS : 内建属性，用于命令行宏定义 -DTEST="test1"
set_property(SOURCE main.cc PROPERTY COMPILE_DEFINITIONS test "TEST=\"test1\"")
```
##### 5. 目标属性
目标属性适合用于传递命令行宏定义
```bash
# 目标test_xlog 必须已经添加了(add_executable)
set_property(TARGET test_xlog PROPERTY COMPILE_DEFINITIONS "TEST1=\"test1\"")
set_property(TARGET test_xlog APPEND PROPERTY COMPILE_DEFINITIONS "TEST2=\"test2\"")
set_property(TARGET test_xlog APPEND PROPERTY COMPILE_DEFINITIONS "TEST3=\"test3\"")
```

```cc
int main (int argc, char *argv[]) {
    xlog x;
#ifdef TEST1
    cout << "TEST1 : " << TEST1 << endl;
#endif

#ifdef TEST2
    cout << "TEST2 : " << TEST2 << endl;
#endif

#ifdef TEST3
    cout << "TEST3 : " << TEST3 << endl;
#endif

    return 0;
}
```

```bash
# cmake --build ./build --target all -v
...
cd /root/cpp_note/07-cmake/05-sub_directory/build/test_xlog && /usr/bin/c++ -DTEST1=\"test1\" -DTEST2=\"test2\" -DTEST3=\"test3\" -I/root/cpp_note/07-cmake/05-sub_directory/test_xlog/../xlog  -MD -MT test_xlog/C
MakeFiles/test_xlog.dir/main.cc.o -MF CMakeFiles/test_xlog.dir/main.cc.o.d -o CMakeFiles/test_xlog.dir/main.cc.o -c /root/cpp_note/07-cmake/05-sub_directory/test_xlog/main.cc
...
```

##### 打印属性 cmake_print_properties
```bash
# 打印属性
include(CMakePrintHelpers)
cmake_print_properties(TARGETS test_xlog  # 目标对象 
                       PROPERTIES         # 属性,可以一次打印多个
                         COMPILE_DEFINITIONS TVAR)
# Properties for TARGET test_xlog:
#   test_xlog.COMPILE_DEFINITIONS = "TEST1="test1";TEST2="test2";TEST3="test3""
#   test_xlog.TVAR = "tvar=11"
```



## 数学运算
```bash
math(EXPR <variable>  # 输出结果保存到变量variable
     "<expression>"   # 数学表达式
     [OUTPUT_FORMAT <format>]) # 输出格式：10/16进制
```

注意：
- 数字类型是64位有符号整数
- 支持计算 : + - * / % | & ^ ~ << >>
- 输出格式
  - HEXADECIMAL : 16进制
  - DECIMAL : 10进制

```bash
set(exp1 "1 + 1")
math(EXPR out1 ${exp1})
message("${exp1} = ${out1}") # 1 + 1 = 2

set(exp2 "2 << 2")
math(EXPR out2 ${exp1} OUTPUT_FORMAT HEXADECIMAL)
message("${exp2} = ${out2}") # 2 << 2 = 0x2
```

## string
### 普通字符串操作
获得子串并转大写
```bash
cmake_minimum_required(VERSION 3.20)

project(string-test)

# 取begin 和 end 之间的子串
set(istr " begin hello world end ")
# 起始标志
set(begin_str "begin")
# 结束标志
set(end_str "end")

# 找到起始标志的下标
string(FIND ${istr} ${begin_str} pos_begin)
# 找到结束标志的下标
string(FIND ${istr} ${end_str} pos_end)

message("pos_begin : ${pos_begin}")
message("pos_end : ${pos_end}")

string(LENGTH ${begin_str} len_begin)

# 调整下标，不包含标志本身
math(EXPR pos_begin "${pos_begin} + ${len_begin}")
math(EXPR pos_end "${pos_end} - 1")

message("pos_begin : ${pos_begin}")
message("pos_end : ${pos_end}")

# 计算子串长度
math(EXPR substring_len "${pos_end} - ${pos_begin}")
# 获得子串
string(SUBSTRING ${istr} ${pos_begin} ${substring_len} ostr)
message("ostr : [${ostr}]")
# 去除两边的空格
string(STRIP ${ostr} ostr)
message("ostr : [${ostr}]")

# 转大写
string(TOUPPER ${ostr} ostr)
message("ostr : [${ostr}]")
```
### json
#### 创建json字符串
```bash
# 使用这种方式可以方便创建多行的字符串的变量 [=[]=]
set(json_str 
[=[
{
    "webs" : {
        "web" : [
            {
                "name" : "cmake",
                "url" : "cmake.org"
            },
            {
                "name" : "ffmpeg",
                "url" : "ffmpeg.org"
            }
        ]
    }
}
]=])
```
#### 读
```bash
#    string(JSON <out-var> [ERROR_VARIABLE <error-var>]
#           {GET | TYPE | LENGTH | REMOVE}
#           <json-string> <member|index> [<member|index> ...])
string(JSON name ERROR_VARIABLE evar
    GET ${json_str} webs web 0 name )
message("name : ${name}") # cmake
message("evar : ${evar}") # NOTFOUND

string(JSON url ERROR_VARIABLE evar
    GET ${json_str} webs web 1 url)
message("url : ${url}") # ffmpeg.org
message("evar : ${evar}") # NOTFOUND

string(JSON url ERROR_VARIABLE evar
    GET ${json_str} webs web 2 url)
message("url : ${url}") # webs-web-2-NOTFOUND
message("evar : ${evar}") # expected an index less than 2 got '2'
```

#### 获得数组长度
```bash
# 2. 获得json数组长度
string(JSON arr_len ERROR_VARIABLE evar
    LENGTH ${json_str} webs web)
message("arr_len : ${arr_len}") # 2
message("evar : ${evar}") # NOTFOUND
```
#### 增加和修改
```bash
# 3. 添加/修改
#  string(JSON <out-var> [ERROR_VARIABLE <error-var>]
#          SET <json-string>
#          <member|index> [<member|index> ...] <value>)
set(new_item 
[=[
{
    "name" : "aa",
    "url" : "aa.org"
}
]=])
string(JSON json_str_new ERROR_VARIABLE evar
    SET ${json_str} webs web ${arr_len} ${new_item})

# {
#   "webs" : 
#   {
#     "web" : 
#     [
#       {
#         "name" : "cmake",
#         "url" : "cmake.org"
#       },
#       {
#         "name" : "ffmpeg",
#         "url" : "ffmpeg.org"
#       },
#       {
#         "name" : "aa",
#         "url" : "aa.org"
#       }
#     ]
#   }
# }
message("json_str_new : ${json_str_new}")

set(new_item
[=[
{
    "name" : "bb",
    "url" : "bb.org",
}
]=])
string(JSON json_str_new ERROR_VARIABLE evar
    SET ${json_str_new} webs web ${arr_len} ${new_item})
# {
#   "webs" : 
#   {
#     "web" : 
#     [
#       {
#         "name" : "cmake",
#         "url" : "cmake.org"
#       },
#       {
#         "name" : "ffmpeg",
#         "url" : "ffmpeg.org"
#       },
#       {
#         "name" : "bb",
#         "url" : "bb.org"
#       }
#     ]
#   }
# }
message("json_str_new : ${json_str_new}")
```
#### 删除
```bash
# 4. 删除
#  string(JSON <out-var> [ERROR_VARIABLE <error-variable>]
#         REMOVE <json-string> <member|index> [<member|index> ...])
#
string(JSON json_str_new ERROR_VARIABLE evar
    REMOVE ${json_str_new} webs web 0)
# {
#   "webs" : 
#   {
#     "web" : 
#     [
#       {
#         "name" : "ffmpeg",
#         "url" : "ffmpeg.org"
#       },
#       {
#         "name" : "bb",
#         "url" : "bb.org"
#       }
#     ]
#   }
# }
message("json_str_new : ${json_str_new}")
```

## 容器
### list
CMake中存储的所有值都是字符串，使用';' 的字符串会被拆分为列表.
```bash
# 1. 创建list
set(l "a" "b" "c" "d;e;f")
message("l : ${l}") # a;b;c;d;e;f

# 2. 获得list长度
list(LENGTH l list_len)
message("list_len : ${list_len}") # 6

# 3. 指定下标获得list项
# 从前往后，下标从0开始
# 从后往前，下标从-1开始
list(GET l 0 l0)
list(GET l 1 l1)
list(GET l -1 ln1)
list(GET l -2 ln2)
message("l0 : ${l0}") # a
message("l1 : ${l1}") # b
message("ln1 : ${ln1}") # f
message("ln2 : ${ln2}") # e

# 4. 追加
list(APPEND l "1;2")
message("l : ${l}") # a;b;c;d;e;f;1;2

# 5. 拼接
list(JOIN l "|" join_list)
message("join_list : ${join_list}") # a|b|c|d|e|f|1|2
list(JOIN l "" join_list)
message("join_list : ${join_list}") # abcdef12
```
除了上述示例操作外，list还提供插入,删除,排序,去重,查找，push, pop等操作。

## 控制语句
### 判断语句 if()
if, elseif, else，本质上是函数，他们可以构成判断语句逻辑

- 语法
```bash
if (<condition>)
    <commands>
elseif (<condition>) # 可选，可重复
    <commands>
else()
    <commands>
endif()
```
- if (<condition>)
  - 表示真：ON, YES, TRUE, Y, 非零
  - 表示假：OFF, NO, FALSE, N, 0, IGNORE, NOTFOUND，空字符串，以NOTFOUND结尾的字符串


- if (<vairable>)
  - 变量直接使用，不需要加$
  - 未定义的变量为假
```bash
if (VAR_NOT_DEF)
    message("VAR_NOT_DEF is true")
else()
    message("VAR_NOT_DEF is false")
endif()


set(VAR_TRUE TRUE)
if (VAR_TRUE)
    message("VAR_TRUE is true")
endif()
```

- if (<string>)
  - 只有特定内容的字符串为true,其他都为false
  - 空字符串为false
```bash
if ("TRUE")
    message("string TRUE is true")
endif()

if ("ON")
    message("string ON is true")
endif()

if ("1234") # 非0，所以为true
    message("string 1234 is true")
endif()

if ("aa")
else()
    message("string aa is false")
endif()
```

- NOT AND OR
  - if (NOT <condition>)
  - if (<cond1> AND <cond2>)
  - if (<cond1> OR <cond2>)
  - if ((cond0) AND (<cond1> OR <cond2>))

- 判断语句
  - 一元判断
    - EXISTS : 文件是否存在
    - COMMAND : 命令是否可用
    - DEFINED : 变量是否定义
```bash
if (NOT DEFINED VAR1)
    message("NOT DEFINED VAR1")
endif()
```
  - 二元判断
    - EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL : 比较数字
```bash
set(VAR1, 123)
if (VAR1 EQUAL 123)
    message("VAR1 == 123")
endif()
```
   - STREQUAL, STRLESS, STRLESS_EQUAL, STRGREATER, STRGREATER_EQUAL : 比较字符串
```bash
set(VAR1, "aaa")
if (VAR1 STREQUAL "aaa")
    message("VAR1 == aaa")
endif()
```

   - VERSION_EQUAL, VERSION_LESS, VERSION_LESS_EQUAL, VERSION_GREATER, VERSION_GREATER_EQUAL : 比较版本号
   - MATCHES : 正则匹配
```bash
if ("abcd123" MATCHES "[a-z]+")
    message("MATCHES")
endif()
```


### 循环语句
#### foreach
##### 语法
```bash
foreach(<loop_var> <items>)
    <commands>
endforeach()
```

foreach支持
- break()
- continue()

##### 指定循环次数

具体有两种
- 指定stop的数值，start为0，step为1，比如stop为3，则循环0，1，2，3
```bash
foreach(<loop_var> RANGE <stop>)
```
- 指定start stop step
```bash
foreach(<loop_var> RANGE <start> <stop> [<step>])
```

##### foreach遍历list
- 1. LISTS关键字
```bash
# lists意味可以有多个list变量，
# foreach 每次取一个元素，将所有list遍历完
foreach(<loop_var> IN [LISTS [<lists>]])
```

- 2. ITEMS
```bash
# items意味着传递list的值(即 ${list})，而非list变量
foreach(<loop_var> IN [ITEMS [<items>]])
```

- 3. ZIP_LISTS
ZIP_LISTS可以同时遍历多个list变量，
比如同时遍历两个list变量，则一次循环，从两个list中分别取一个元素。
为了区分元素，有两种方法：
  - 1）用户定义loop_var，cmake根据loop_var名称添加后缀_x，生成对应数量的变量
```bash
foreach(num IN ZIP_LISTS list1 list2)
   message("num_0 : ${num_0}")
   message("num_1 : ${num_1}")
endforeach()
```
  * 2）用户定义对应数量的变量
```bash
foreach(v1 v2 IN ZIP_LISTS list1 list2)
    message("v1 : ${v1}")
    message("v2 : ${v2}")
endforeach()
```

#### while
##### 语法
```bash
# condition为真时，循环
while(<condition>)
    <commands>
endwhile()
```

## 宏
### 语法
宏的定义
```bash
macro(<name> [<arg1> ...])
    <commands>
endmacro()
```

宏的使用
宏大小写不敏感
```bash
foo()
Foo()
FOO()
cmake_language(CALL foo)
```

### 参数
#### 普通方式传递参数
用户可以显示定义参数
```bash
marco(foo arg1 arg2)
    message("arg1 : ${arg1}")
    message("arg2 : ${arg2}")
endmarco()
```

也可以不定义参数名，直接传递参数
此时，通过关键字访问参数:
- ARGC : 所有参数个数
- ARGN : 除了预定义参数外，通过可变参传递的参数
- ARGV0 ARGV1 ARGV2 : 访问预定义的参数
```bash
marco(foo)
    message("ARGC: ${ARGC}")
    message("ARGN: ${ARGN}")
endmarco()
```

```bash
marco(foo arg1 arg2)
    message("ARGC: ${ARGC}")
    message("ARGN: ${ARGN}")
    message("ARGV0: ${ARGV0}")
    message("ARGV1: ${ARGV1}")
endmarco()

# ARGC : 5
# ARGN : 1;2;3
# ARGV0 : a
# ARGV1 : b
marco("a" "b" "1" "2" "3")

# ARGC : 2
# ARGN : 
# ARGV0 : a
# ARGV1 : b
marco("a" "b")
```

> [!NOTE]
> 参数不是变量，因为宏的参数是替换的标志符，所以以下代码都是错误的
```bash
if (ARGV1)
foreach(loop_var IN LISTS ARGN)
```
必须取值使用
```bash
if (${ARGV1})
foreach(loop_var IN LISTS ${ARGN})
```
#### 传递并解析参数
用户使用 cmake_parse_arguments 设置宏接受的参数的解析方式，以实现精准控制参数传递
```bash
cmake_parse_arguments(<prefix>  # 前缀值，用于给参数名加前缀，避免名称冲突
                      <options> # bool类型的参数
                      <one_value_keywords> # 单个value类型的参数
                      <multi_value_keywords>  # 多个value类型的参数
                      <args>...) # 需要解析的参数数组
```

示例
```bash
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
```

为了方便调试，cmake_parse_arguments还生成了描述传递参数错误和未传递参数的错误信息

```bash
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

    # 打印错误信息
    message("MY_UNPARSED_ARGUMENTS : ${MY_UNPARSED_ARGUMENTS}")
    message("MY_KEYWORDS_MISSING_VALUES : ${MY_KEYWORDS_MISSING_VALUES}")
endmacro()


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

```
## 函数
语法
```bash
function(<name> [<arg1> ...])
    <commands>
endfunction()
```

### 函数和宏对比
- 函数和宏的区别
  - 函数的参数是变量，宏的参数是替换
  - 函数内部的变量作用域是函数内

  - 函数可以return
- 函数和宏的相同
  - 传递参数的方式相同
    - 可以预定义参数名
    - 可以不预定参数名，传递可变长度的参数
    - 可以用 cmake_parse_arguments 解析参数

### 函数作用域的问题
- 函数内set的变量，作用域是函数
- 如果希望函数内set变量是全局变量，需要加 PARENT_SCOPE

```bash
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
```

# 生成表达式
生成表达式是在生成阶段执行的，
```bash
cmake --build ./build --target all
```
之前讲的内容都是在配置阶段执行的
```bash
cmake -S . -B build
```

## 简单示例
```bash
cmake_minimum_required(VERSION 3.20)
project(generator-exp-test)

add_executable(a.out main.cc)

# target_compile_definitions 用于添加 -D 到目标
# $<$<BOOL:ON>:TEST1=123> 就是生成表达式，
# $<BOOL:ON> 为bool表达式，返回true/false
# 外部是条件表达式 $<condition:true_string> : 
#    当condition为真时，返回true_string
target_compile_definitions(a.out PUBLIC "$<$<BOOL:ON>:TEST1=123>")
```
```cc
#include <iostream>
using namespace std;

int main (int argc, char *argv[]) {
#ifdef TEST1
    cout << TEST1 << endl;
#endif
    
    return 0;
}
```
执行配置时，生成表达式不会执行，只是记录。
当生成目标时，生成表达式会执行，这里会添加 "-DTEST1=123"

## 调试生成表达式
利用cmake错误来查看生成表达式的值

```bash
set(LIB OFF)
# 这里会报错，停止生成阶段，并执行 $<$<NOT:$<BOOL:${LIB}>>:static> 打印 STATIC
target_include_directories(a.out PUBLIC "$<$<NOT:$<BOOL:${LIB}>>:static>")
```

官方给出的调试方法
```bash
add_custom_target(gen COMMAND 
    ${CMAKE_COMMAND} -E echo "$<$<NOT:$<BOOL:${LIB}>>:STATIC>")
```
生成阶段可以build自定义目标，以显示生成表达式的值
```bash
❯ cmake --build ./build --target gen
STATIC
Built target gen
```
## 各种类型表达式
### 逻辑运算符, 返回0/1
```bash
$<BOOL:string> # 根据string返回0/1
$<NOT:condition> # condition为0，返回1，为1返回0
$<AND:condition1,condition2> # 返回 condition1 && condition2 
$<OR:condition1,condition2> # 返回 condition1 || condition2
```

### 条件表达式
根据condition为0/1，返回string
```bash
$<condition:true string> # condition为0返回空串，为1返回true string
$<IF:condition,true_string,false_string> # condition为0返回false_string，为1返回true string
```

```bash
set(LIB ON)
add_custom_target(gen_if COMMAND
    ${CMAKE_COMMAND} -E echo "$<IF:$<BOOL:${LIB}>,ON_STRING,OFF_STRING>")
```
### 字符串比较
```bash
set(LIB OFF)
target_include_directories(a.out PUBLIC
    "$<STREQUAL:${LIB},OFF>")

target_include_directories(a.out PUBLIC
    "$<EQUAL:123,123>")
```

字符串大小写转换
```bash
$<LOWER_CASE:string>
$<UPPER_CASE:string>
```

### 变量查询
```bash
add_custom_target(gen_config COMMAND
    ${CMAKE_COMMAND} -E echo "$<CONFIG>")

add_custom_target(gen_platform COMMAND
    ${CMAKE_COMMAND} -E echo "$<PLATFORM_ID:Windows,Linux>") # 查询PLATFORM_ID变量中是否有Windows或Linux字符串值，返回1/0
```

### 目标相关
查询一个目标的相关值。
目标可以是 add_executable add_library 引入的执行文件或库文件
```bash
$<TARGET_NAME_IF_EXISTS:target>
$<TARGET_FILE:target>
$<TARGET_PDB_FILE:target>
$<TARGET_PROPERTY:target,property>
```

示例
```bash
# build时获得目标a.out的全路径
# /root/cpp_note/07-cmake/11-generation-exp/build/a.out
set(exp "$<TARGET_FILE:a.out>") 

# build时获得目标a.out的名称
# a.out
set(exp "$<TARGET_PROPERTY:a.out,NAME>") 

add_custom_target(gen_target COMMAND
    ${CMAKE_COMMAND} -E echo "${exp}")
```

# 配置构建参数

## 头文件路径 和 链接选项
```bash
 target_include_directories(<target> [SYSTEM] [AFTER|BEFORE]
                                  <INTERFACE|PUBLIC|PRIVATE> [items1...]
                                  [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
```
对某个目标添加头文件搜索目录。
- target : 由 add_library / add_executable 添加的目标
- SYSTEM : 告诉编译器此路径是系统路径，解决平台警告的问题
- AFTER|BEFORE : 添加在变量的头部还是尾部，解决优先级问题
- 继承参数 : 用于依赖情况，比如目标是库，那么依赖他的其他目标是否继承此路径
  - INTERFACE : 只依赖者使用，改变 INTERFACE_INCLUDE_DIRECTORIES
  - PUBLIC :  依赖者和自己都使用，改变 INTERFACE_INCLUDE_DIRECTORIES , INCLUDE_DIRECTORIES 两个变量
  - PRIVATE : 只自己使用, 改变 INCLUDE_DIRECTORIES 变量


```bash
 target_compile_definitions(<target>
   <INTERFACE|PUBLIC|PRIVATE> [items1...]
   [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
```
设置编译宏定义

```bash
target_link_libraries(<target> 
                          <INTERFACE|PUBLIC|PRIVATE> [items1...]
                          [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
```
- 相当于添加gcc的 -lxxx 信息
- target : 可以是可执行文件名也可以是库
- 继承参数：
  - INTERFACE : 当前目标不使用添加的信息，依赖者使用
  - PUBLIC : 当前目标使用添加信息，依赖者也使用
  - PRIVATE : 当前目标使用

---

下面的示例展示 target_include_directories target_link_libraries target_compile_definitions 的用法
```bash
# 创建文件
file(WRITE a.cpp [=[
    void A() {
    }
]=])

add_library(A STATIC a.cpp)

# 修改变量 A.INCLUDE_DIRECTORIES  A.INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(A PUBLIC "/A_PUBLIC")
# 修改变量 A.INCLUDE_DIRECTORIES
target_include_directories(A PRIVATE "/A_PRIVATE")
# 修改变量 A.INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(A INTERFACE "/A_INTERFACE")

include(CMakePrintHelpers)

# 打印结果
# Properties for TARGET A:
#   A.INCLUDE_DIRECTORIES = "/A_PUBLIC;/A_PRIVATE"
#   A.INTERFACE_INCLUDE_DIRECTORIES = "/A_PUBLIC;/A_INTERFACE"
cmake_print_properties(TARGETS A PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES)

file(WRITE b.cpp [=[
    void A();
    void B() {
        A();
    }
    ]=])

add_library(B STATIC b.cpp)
# B -> A
target_link_libraries(B PRIVATE A)

# 不能打印继承的属性
cmake_print_properties(TARGETS B PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES)

# 查询B的 include相关属性值
# 只能通过cmake原生调试方法打印继承的属性
# Used includes for target B:
#  * /A_PUBLIC
#  * /A_INTERFACE
set(CMAKE_DEBUG_TARGET_PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES)

file(WRITE main.cpp [=[
    void B();
    int main() {
        B();
        return 0;
    }
    ]=])

add_executable(a.out main.cpp)
# a.out -> B -> A
target_link_libraries(a.out PRIVATE B)
```
---

## add_library
### obj文件和so文件分步编译链接
```bash
cmake_minimum_required(VERSION 3.22)
project(link-obj)

file(WRITE a.cpp [=[
    #include <iostream>
    void A() {
    std::cout << "A " << std::endl;
    }
    ]=])

file(WRITE b.cpp [=[
    #include <iostream>
    void B() {
    std::cout << "B " << std::endl;
    }
    ]=])


file(WRITE main.cpp [=[
    #include <iostream>
    int main() {
     A();
     B();
     return 0;
    }
    ]=])

# 生成虚拟对象testab_obj，用于将 a.cpp b.cpp -> a.cpp.o b.cpp.o
add_library(testab_obj OBJECT a.cpp b.cpp)
# 最终生成动态库所以编译时需要加 fPIC
if (NOT MSVC)
    target_compile_options(testab_obj PRIVATE "-fPIC")
endif()

# 错误找不到 testab_obj
# add_library(testab testab_obj)

# add_library SHARED 会自动添加  -fPIC
# 必须用生成表达式将 $<TARGET_OBJECTS:testab_obj> -> a.cpp.o b.cpp.o
add_library(testab SHARED $<TARGET_OBJECTS:testab_obj>)
```

### 指定软件版本
```bash
cmake_minimum_required(VERSION 3.22)
project(lib-version)

file(WRITE a.cc  [=[
void A() {}
]=])

file(WRITE main.cc  [=[
int main() {
    return 0;
}
]=])

# 带版本的库只在SO文件中有意义
add_library(a SHARED a.cc)
# 设置a库的版本为 2.0.1 
# SOVERSION ： 更细的版本
# NO_SONAME : ON 不生成带版本的库，OFF生成带版本的库
set_target_properties(a PROPERTIES 
    VERSION "2.0.1"
    SOVERSION "3"
    NO_SONAME OFF
)

add_executable(main main.cc)
target_link_libraries(main a)
# 可执行程序也可以设置软件版本
set_target_properties(main PROPERTIES
    VERSION "2.0.1")
```
构建结果
```bash
lrwxrwxrwx 1 root root     9  7月 10 15:15 liba.so -> liba.so.3
-rwxr-xr-x 1 root root 15104  7月 10 15:15 liba.so.2.0.1
lrwxrwxrwx 1 root root    13  7月 10 15:15 liba.so.3 -> liba.so.2.0.1
lrwxrwxrwx 1 root root    10  7月 10 15:15 main -> main-2.0.1
-rwxr-xr-x 1 root root 15776  7月 10 15:15 main-2.0.1
```

### 指定debug release minsize 的编译选项
```bash
# 创建静态库源文件
file(WRITE src/slib.cpp [=[
    void Slib() {
    }
    ]=])

# CMAKE_BUILD_TYPE : 控制编译优化等级和debug参数的变量
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 用户可以在配置时期设置 CMAKE_BUILD_TYPE
# cmake -S . -B build  -D CMAKE_BUILD_TYPE=xxx
# 设置不同值时，cmake时机采用的编译参数:
#     Debug: 
#          -g
#     Release:
#          -O3 -DNDEBUG 
#     RelWithDebInfo:
#          -O2 -g -DNDEBUG
#     MinSizeRel:
#          -Os -DNDEBUG

add_library(slib src/slib.cpp)
```
### 指定文件的输出路径
```bash
# 创建静态库源文件
file(WRITE src/slib.cpp [=[
    void Slib() {
    }
    ]=])

# CMAKE_BUILD_TYPE : 控制编译优化等级和debug参数的变量
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_library(slib STATIC src/slib.cpp)

set(LIB_OUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(EXE_OUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# 设置不同类别的静态库slib的输出路径
set_target_properties(slib PROPERTIES 
    # 设置静态库的输出路径
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release
)

file(WRITE include/dlib.h [=[
    void dlib();
    ]=])
file(WRITE src/dlib.cpp [=[
    #include "dlib.h"
    void dlib() {}
    ]=])


add_library(dlib SHARED src/dlib.cpp include/dlib.h)
target_include_directories(dlib PUBLIC include)
set_target_properties(dlib PROPERTIES
    # 设置静态库的输出路径
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release

    # win : 设置dll的输出路径
    RUNTIME_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release

    # linux/mac : 设置.so的输出路径
    LIBRARY_OUTPUT_DIRECTORY ${LIB_OUT_PATH}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_PATH}/debug
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_PATH}/release
)

file(WRITE src/main.cpp [=[
    #include "dlib.h"
    void Slib();
    int main() {
    Slib();
    dlib();
    }
    ]=])

add_executable(main src/main.cpp)

# 指定bin文件的输出路径
target_link_libraries(main slib dlib)
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${EXE_OUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXE_OUT_PATH}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXE_OUT_PATH}/release)
```
# install部署项目

## 目标安装
### 按照target类别安装
```bash
install(TARGETS <target>... [...])
```
- target: 被安装的目标，由add_library , add_executable 创建
- target的类别:
  - RUNTIME : 执行程序（由add_executable创建）, Windows动态链接库文件(dll)
  - ARCHIVE : 静态库文件(add_library STATIC创建), Windows是.lib ，Linux是.a
  - LIBRARY : 动态库文件(add_library SHARED创建), linux .so 
  - PRIVATE_HEADER, PUBLIC_HEADER : 私有头文件，公开头文件，（由 set_target_properties(slib PROPERTIES PUBLIC_HEADER/PRIVATE_HEADER file.h)）

示例
```bash
#---------------------------------------------------------------------
# 统一安装所目标
# 以 ${CMAKE_INSTALL_PREFIX} 为当前目录，
# 结合 DESTINATION 指定的目录 获得安装目录
# 比如下面是 
# ${CMAKE_INSTALL_PREFIX}/bin
message("CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX}")
# install(TARGETS slib dlib main DESTINATION bin)

# cmake -S . -B build  -DCMAKE_INSTALL_PREFIX=./install
# 执行安装 cmake --install ./build
# install
# └── bin
#     ├── libdlib.so
#     ├── libslib.a
#     └── main

# 默认CMAKE_INSTALL_PREFIX 是 /usr/local/


#---------------------------------------------------------------------
# 分类安装

# 设置 slib 的私有头文件和共有头文件
set_target_properties(slib PROPERTIES PUBLIC_HEADER include/slib.h)
set_target_properties(slib PROPERTIES PRIVATE_HEADER include/slib_pri.h)

install(TARGETS slib dlib main # 需要安装的目标
    RUNTIME DESTINATION bin # 可执行程序
    ARCHIVE DESTINATION lib # 静态库
    LIBRARY DESTINATION libso # 动态库
    PRIVATE_HEADER DESTINATION inc_pri # 私有头文件
    PUBLIC_HEADER DESTINATION include  # 共有头文件
)

# 安装结果
# ❯ cmake --install ./build
# -- Install configuration: ""
# -- Up-to-date: /root/cpp_note/07-cmake/16-install/install/lib/libslib.a
# -- Installing: /root/cpp_note/07-cmake/16-install/install/inc_pri/slib_pri.h
# -- Installing: /root/cpp_note/07-cmake/16-install/install/include/slib.h
# -- Up-to-date: /root/cpp_note/07-cmake/16-install/install/libso/libdlib.so
# -- Up-to-date: /root/cpp_note/07-cmake/16-install/install/bin/main

```

### 区分Debug和Release版本
```bash
#---------------------------------------------------------------------
# 区分debug和release版本
install(TARGETS slib dlib main
    CONFIGURATIONS Debug    # debug版本 : 
                            # 只有 cmake --install ./xxx --config Debug 
                            # 才会触发这个install
    RUNTIME DESTINATION debug/bin # 可执行程序
    ARCHIVE DESTINATION debug/lib # 静态库
    LIBRARY DESTINATION debug/libso # 动态库
    PRIVATE_HEADER DESTINATION debug/inc_pri # 私有头文件
    PUBLIC_HEADER DESTINATION debug/include  # 共有头文件
)

install(TARGETS slib dlib main
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel    # debug版本
    RUNTIME DESTINATION release/bin # 可执行程序
    ARCHIVE DESTINATION release/lib # 静态库
    LIBRARY DESTINATION release/libso # 动态库
    PRIVATE_HEADER DESTINATION release/inc_pri # 私有头文件
    PUBLIC_HEADER  DESTINATION release/include  # 共有头文件
)
```
编译debug并安装
```bash
# CMAKE_BUILD_TYPE=Debug : 决定Debug方式编译
cmake -S . -B build  -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build
# 执行Debug版本的install
cmake --install ./build --config Debug
```
编译Release并安装
```bash
# CMAKE_BUILD_TYPE=Release : 决定Release方式编译
cmake -S . -B build  -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
# 执行Release版本的install
cmake --install ./build --config Release
```
## 文件安装
### 通用文件安装
用户直接指定安装的目录
```bash
# 将文件安装到指定目录
install(FILES a.h b.h DESTINATION include)
# 添加OPTIONAL : 文件可以不存在
install(FILES e.h  DESTINATION include OPTIONAL)
```
```bash
❯ cmake --install ./build
-- Install configuration: ""
-- Installing: /root/cpp_note/07-cmake/17-install-file/install/include/a.h
-- Installing: /root/cpp_note/07-cmake/17-install-file/install/include/b.h
```
### 按照GNU规则安装
用户指定文件的类型，按照GNU规范的目录进行安装。
```bash
# 根据GNU规则将文件按照类型，安装到约定目录
include(GNUInstallDirs)
message("CMAKE_INSTALL_DATAROOTDIR : ${CMAKE_INSTALL_DATAROOTDIR}")
install(FILES a.h TYPE DOC)
install(FILES b.h TYPE LIB)
install(FILES c.h TYPE INCLUDE)
```
```bash
❯ cmake --install ./build
-- Install configuration: ""
-- Installing: /root/cpp_note/07-cmake/17-install-file/install/share/doc/install-file/a.h
-- Installing: /root/cpp_note/07-cmake/17-install-file/install/lib/b.h
-- Installing: /root/cpp_note/07-cmake/17-install-file/install/include/c.h
```

### 文件权限
```bash
# 文件权限
install(FILES a.h DESTINATION pub
    PERMISSIONS 
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_EXECUTE
    WORLD_READ WORLD_WRITE WORLD_EXECUTE)
```

## 目录安装
### 基本的目录安装
```bash
file(WRITE doc/index.html "")
file(WRITE doc/index.htm "")
file(WRITE doc/doc.cc "")
file(WRITE doc/doc.c "")
file(WRITE doc/sub/doc.c "")

# 安装指定目录下所有文件，包含子目录
# 指定文件类型，安装到相关类型的默认目录
install(DIRECTORY doc TYPE DOC)
# 指定安装目录
install(DIRECTORY doc DESTINATION doc2)
```
```bash
❯ cmake -S . -B build  -DCMAKE_INSTALL_PREFIX=./install
❯ cmake --install ./build
-- Install configuration: ""
-- Up-to-date: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc/index.html
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc/doc.cc
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc/sub
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc/sub/doc.c
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc/doc.c
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/share/doc/doc/index.htm
-- Up-to-date: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc/index.html
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc/doc.cc
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc/sub
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc/sub/doc.c
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc/doc.c
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/doc2/doc/index.htm
```
### 过滤安装指定模式的文件
```bash
file(WRITE doc/index.html "")
file(WRITE doc/index.htm "")
file(WRITE doc/doc.cc "")
file(WRITE doc/doc.c "")
file(WRITE doc/sub/doc.c "")
file(WRITE doc/sub/index.html "")

# 只安装后缀为 html htm的文件
install(DIRECTORY doc DESTINATION html
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.htm")
```
```bash
❯ cmake --install ./build
-- Install configuration: ""
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/index.html
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/sub
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/sub/index.html
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/index.htm
```
### 反向过滤
```bash
file(WRITE doc/index.html "")
file(WRITE doc/index.htm "")
file(WRITE doc/doc.cc "")
file(WRITE doc/doc.c "")
file(WRITE doc/sub/doc.c "")
file(WRITE doc/sub/index.html "")
file(WRITE doc/.git/index.html "")
file(WRITE doc/.svn/index.html "")
file(WRITE doc/sub/.svn "")
file(WRITE doc/sub/.git "")

# 安装后缀为html html的文件
# 但是过滤掉.git .svn目录和文件
install(DIRECTORY doc DESTINATION html
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.htm"
    PATTERN ".git" EXCLUDE
    PATTERN ".svn" EXCLUDE)
```
```bash
❯ cmake --install ./build
-- Install configuration: ""
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/index.html
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/sub
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/sub/index.html
-- Installing: /root/cpp_note/07-cmake/18-install-doc/install/html/doc/index.htm
```

## 安装期间执行命令
常用于打印日志
```bash
install(CODE "message(\"1111\")")

install(DIRECTORY doc DESTINATION html
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.htm"
    PATTERN ".git" EXCLUDE
    PATTERN ".svn" EXCLUDE)

install(CODE "message(\"222\")")

install(CODE [=[
    string(TIMESTAMP now "%Y-%m-%d %H:%M:%S")
    message(${now})
    FILE(APPEND install_log.txt "${now}\n")
    ]=])
```

## package
### 相关函数

#### 导出package
分两步:
- 1. 在安装目标时添加EXPORT 
```bash
install(TARGETS slib dlib main # 需要安装的目标

    EXPORT slib # 要导出slib相关内容的安装信息

    RUNTIME DESTINATION bin # 可执行程序
    ARCHIVE DESTINATION lib # 静态库
    LIBRARY DESTINATION libso # 动态库
    PRIVATE_HEADER DESTINATION inc_pri # 私有头文件
    PUBLIC_HEADER DESTINATION include  # 共有头文件
)
```
- 2. 安装导出文件
```bash
install(EXPORT slib  # 指定要导出的目标
    NAMESPACE xcpp:: # 可选，添加命名空间
    FILE slibConfig.cmake  # 导出的Config文件名
    DESTINATION mod/slib/  # 导出到哪里
    )
```
#### 使用package
```bash
find_package(<PackageName> [version] [EXACT] [QUIET] [MODULE]
              [REQUIRED] [[COMPONENTS] [components...]]
              [OPTIONAL_COMPONENTS components...]
              [REGISTRY_VIEW  (64|32|64_32|32_64|HOST|TARGET|BOTH)
```
find_package返回 `<PackageName>_FOUND` 变量.
使用package可以自动获得依赖库的头文件和库文件路径.

cmake 会在 CMAKE_PREFIX_PATH 变量存储的路径下去找 config文件 和 version文件
- config文件
  - 描述库的头文件路径和库路径
  - 名称格式： `<PackageName>-config.cmake`  或 `<PackageName>Config.cmake`
- version文件
  - 描述库的版本信息，和版本兼容方式
  - 名称格式： `<PackageName>-config-version.cmake`  或 `<PackageName>ConfigVersion.cmake`


示例
```bash
# cmake会去找slib的Config.cmake文件
# 其中定义了slib目标的 INTERFACE_INCLUDE_DIRECTORIES 等信息
find_package(slib)
add_executable(main main.cc)
# 由于main依赖slib，所以main会继承slib的 INTERFACE_INCLUDE_DIRECTORIES等信息
# 所以不需要手动指定 target_include_directories 也能找到正确的头文件路径
target_link_libraries(main slib)
```

### 示例1 不考虑version
#### 导出package Config
```bash
cmake_minimum_required(VERSION 3.22)
project(package-test)

file(WRITE include/slib.h "void slib();")
file(WRITE slib.cc [=[
    #include <iostream>
    #include "slib.h"
    using namespace std;
    void slib() {
    cout << "slib" << endl;
    }
    ]=])

add_library(slib STATIC slib.cc include/slib.h)

set_target_properties(slib PROPERTIES PUBLIC_HEADER include/slib.h)

# 不能这样写，因为 
# ${CMAKE_CURRENT_SOURCE_DIR}/include 是编译库时有效，
# 而Config文件中应该记录 install_dir/include 
#
# target_include_directories(slib PUBLIC include)

# 所以必须写成下面形式
target_include_directories(slib PUBLIC 
    # build时有效，install时为空
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    # install时有效，build时为空
    $<INSTALL_INTERFACE:include>
)

install(TARGETS slib

    # 生成config信息
    EXPORT slib

    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

# 安装config信息
install(EXPORT slib
    FILE slibConfig.cmake 
    DESTINATION config
)
```

```bash
❯ cmake -S . -B build
❯ cmake --build build -v
❯ cmake --install build --prefix=../out
-- Install configuration: ""
-- Installing: /root/cpp_note/07-cmake/23-package/export-package/../out/lib/libslib.a
-- Installing: /root/cpp_note/07-cmake/23-package/export-package/../out/include/slib.h
-- Up-to-date: /root/cpp_note/07-cmake/23-package/export-package/../out/config/slibConfig.cmake
-- Up-to-date: /root/cpp_note/07-cmake/23-package/export-package/../out/config/slibConfig-noconfig.cmake
```
#### 使用package Config
```bash
cmake_minimum_required(VERSION 3.22)
project(find_package-test)

file(WRITE main.cc [=[
    #include "slib.h"
    int main() {
    slib();
    return 0;
    }
    ]=])

# cmake 根据 CMAKE_PREFIX_PATH 变量查找slib的Config
# 返回 slib_FOUND 显示是否找到
find_package(slib)
if (slib_FOUND)
    message("slib found success!")
endif()
# Config中定义了slib目录，slib的的属性有 
# INTERFACE_INCLUDE_DIRECTORIES 记录了头文件的路径
get_target_property(inc slib INTERFACE_INCLUDE_DIRECTORIES)
message("inc : ${inc}")

add_executable(main main.cc)

# 由于main依赖slib
# 所以继承 slib的 INTERFACE_INCLUDE_DIRECTORIES
# 所以能找到slib的公共头文件
target_link_libraries(main slib)
```

```bash
❯ cmake -S . -B build  -DCMAKE_PREFIX_PATH=../out/config
slib found success!
inc : /root/cpp_note/07-cmake/23-package/out/include
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /root/cpp_note/07-cmake/23-package/find-package/build
```

### 示例2 考虑version
#### 生成 package Config 和 version
```bash
cmake_minimum_required(VERSION 3.22)
project(package-test)

# version默认为1.0
# 用户可以通过 -Dversion=xxx 修改
if (NOT version)
    set(version 1.0)
endif()

file(WRITE include/slib.h "void slib();")

# 模板代码
file(WRITE slib.cc.in [=[
    #include <iostream>
    #include "slib.h"
    using namespace std;
    void slib() {
    cout << "slib ${version}" << endl;
    }
    ]=])

# 会将 slib.cc.in 的内容进行替换,
# 这里会将 ${version} 替换为 具体的版本号
# 输出到文件 slib.cc
configure_file(slib.cc.in "${CMAKE_CURRENT_SOURCE_DIR}/slib.cc")

add_library(slib STATIC slib.cc include/slib.h)

set_target_properties(slib PROPERTIES PUBLIC_HEADER include/slib.h)

# 不能这样写，因为 
# ${CMAKE_CURRENT_SOURCE_DIR}/include 是编译库时有效，
# 而Config文件中应该记录 install_dir/include 
#
# target_include_directories(slib PUBLIC include)

# 所以必须写成下面形式
target_include_directories(slib PUBLIC 
    # build时有效，install时为空
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    # install时有效，build时为空
    $<INSTALL_INTERFACE:${version}/include>
)

install(TARGETS slib
    # 生成config信息
    EXPORT slib

    RUNTIME DESTINATION ${version}/bin
    ARCHIVE DESTINATION ${version}/lib
    LIBRARY DESTINATION ${version}/lib
    PUBLIC_HEADER DESTINATION ${version}/include
)

# 安装config信息
install(EXPORT slib
    FILE slibConfig.cmake 
    DESTINATION config/slib-${version}/
)

# 注意：在 cmake 配置阶段就会执行，
# 所以一定要设置 CMAKE_INSTALL_PREFIX
#
# 写入版本信息 
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    # 文件路径
    ${CMAKE_INSTALL_PREFIX}/config/slib-${version}/slibConfigVersion.cmake
    # 版本
    VERSION ${version}
    # 版本兼容方式，主版本号兼容
    COMPATIBILITY SameMajorVersion
)
```

```bash
❯ cmake -S . -B build -DCMAKE_INSTALL_PREFIX=../out/
❯ cmake --build build
❯ cmake --install build
```

#### 使用 package Config 和 version
```bash
cmake_minimum_required(VERSION 3.22)
project(find_package-test)

file(WRITE main.cc [=[
    #include "slib.h"
    int main() {
    slib();
    return 0;
    }
    ]=])

if (NOT version)
    set(version 1.0)
endif()

# cmake 根据 CMAKE_PREFIX_PATH 变量查找slib的Config
# 返回 slib_FOUND 显示是否找到
# 需要添加 version 参数
find_package(slib ${version})
if (slib_FOUND)
    message("slib found success!")
endif()
# Config中定义了slib目录，slib的的属性有 
# INTERFACE_INCLUDE_DIRECTORIES 记录了头文件的路径
get_target_property(inc slib INTERFACE_INCLUDE_DIRECTORIES)
message("inc : ${inc}")

add_executable(main main.cc)

# 由于main依赖slib
# 所以继承 slib的 INTERFACE_INCLUDE_DIRECTORIES
# 所以能找到slib的公共头文件
target_link_libraries(main slib)
```

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=../out/config
```

# 交叉编译
## 重要变量
- CMAKE_SYSTEM_NAME
  - 目标操作系统
  - Linux/Windows/Generic(无系统，单片机)
- CMAKE_SYSTEM_PROCESSOR
  - 目标系统的处理器名称
- CMAKE_C_COMPILER
  - C编译器全路径
- CMAKE_CXX_COMPILER
  - 目标环境C++编译器全路径
  - GNU工具链只需要设置C编译器全路径，cmake会自动查找C++编译器路径
- CMAKE_SYSROOT
  - 系统库，头文件的路径
- CMAKE_TOOLCHAIN_FILE

## 示例
```bash
# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(cross-compile-test)

file(WRITE main.cc [=[
    int main() {
    return 0;
    }
    ]=])

add_executable(main main.cc)
```
```bash
# linux_arm_toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabi-g++)
```

执行
```bash
cmake -S . -B arm -DCMAKE_TOOLCHAIN_FILE=./linux_arm_toolchain.cmake
cmake --build ./arm -v
```

# 单元测试
## ctest
### 涉及函数

```bash
 add_test(NAME <name> COMMAND <command> [<arg>...]
          [CONFIGURATIONS <config>...]
          [WORKING_DIRECTORY <dir>]
          [COMMAND_EXPAND_LISTS])
```
添加一条测试。
name : 用于控制台显示，以区分不同的测试
command : 测试指令

```bash
enable_testing()
```
用于启用当前目录及其子目录的测试。

### 根据程序返回值测试
```bash
file(WRITE main.cc [=[
    #include <iostream>
    int main(int argc, char **argv) {
    if (argc <= 1) {
        return -1;
        }
        std::cout << argv[1] << std::endl;
        return 0;
    }
    ]=])

add_executable(main main.cc)

# 添加一个单元测试，执行指令 main
# 期望返回-1，导致ctest识别为测试失败
add_test(NAME test_return_-1
    COMMAND main)

# 添加一个单元测试，执行指令 main aa
# 期望返回0，导致ctest识别为测试成功
add_test(NAME test_return_0
    COMMAND main aa)

# 开启当前目录和子目录的测试
enable_testing()
```

执行单元测试
```bash
# 正常构建
❯ cmake -S . -B build
❯ cmake --build build -v

# 进入build目录，执行ctest指令
❯ cd build
❯ ctest -C Debug
Test project /root/cpp_note/07-cmake/20-ctest/build
    Start 1: test_return_-1
1/2 Test #1: test_return_-1 ...................***Failed    0.00 sec
    Start 2: test_return_0
2/2 Test #2: test_return_0 ....................   Passed    0.00 sec

50% tests passed, 1 tests failed out of 2

Total Test time (real) =   0.00 sec

The following tests FAILED:
          1 - test_return_-1 (Failed)
Errors while running CTest
Output from these tests are in: /root/cpp_note/07-cmake/20-ctest/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
``` 

### 根据程序输出测试
涉及函数
```bash
 set_tests_properties(<tests>...
                      [DIRECTORY <dir>]
                      PROPERTIES <prop1> <value1>
                      [<prop2> <value2>]...)
```
示例
```bash
file(WRITE main.cc [=[
    #include <iostream>
    int main(int argc, char **argv) {
    if (argc <= 1) {
        return -1;
        }
        std::cout << argv[1] << std::endl;
        return 0;
    }
    ]=])

add_executable(main main.cc)

# 添加测试，执行 main aaa
add_test(NAME test_ok
    COMMAND main aaa)
# 根据程序输出进行测试
set_tests_properties(test_ok
    PROPERTIES
    PASS_REGULAR_EXPRESSION aaa) # 输出结果有aaa，则为PASS，否则为FAIL

add_test(NAME test_fail
    COMMAND main bbb)
set_tests_properties(test_fail
    PROPERTIES
    FAIL_REGULAR_EXPRESSION bbb) # 输出结果有bbb，则为FAIL，否则为PASS

enable_testing()
```

## gtest
### gtest的安装 
#### 官方推荐方法
```bash
cmake_minimum_required(VERSION 3.10)
project(test_gtest)

# 1. 安装gtest
# 1.1 下载googletest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL 
    https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
)

# 1.2 编译安装googletest
FetchContent_MakeAvailable(googletest)

```
执行安装
```bash
#  执行下述指令完成gtest的编译安装
#  1. 下载gtest
cmake -S . -B build
#  2. 编译gtest
cmake --build build -v
#  3. 安装gtest
cmake --install build --prefix=./
```
使用官方推荐方法的好处是：
1. 会自动检查gtest是否已安装，避免重复执行
2. 自动添加gtest目标，方便后续引用


使用gtest官方方法的完整示例:
```bash
cmake_minimum_required(VERSION 3.10)
project(test_gtest)

# 1. 安装gtest
# 1.1 下载googletest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL 
    https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
)

# 1.2 编译安装googletest
FetchContent_MakeAvailable(googletest)

# 2. 准备gtest测试程序
# 2.1
# main.cc 单元测试代码
add_executable(main main.cc)
# 设定链接库，头文件自动推导
target_link_libraries(main PUBLIC GTest::gtest_main)

# 2.2
# 将gtest测试程序main，将他包装成ctest
include(GoogleTest)
gtest_discover_tests(main)

# 3.
# 启动测试
enable_testing()
```
gtest测试程序
```cc
#include <gtest/gtest.h>

TEST(MyTest, MyTestDownload) {
    EXPECT_EQ(4*4, 16);
    EXPECT_EQ(4*4, 15);
}

int main (int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```
执行测试
```bash
cmake -S . -B build
cmake --build ./build
cd build
ctest
```

#### 手动安装

使用execute_process函数执行指令进行手动安装

```bash
# 解压
# cd ${PROJECT_BINARY_DIR}
# cmake -E tar xf ${CMAKE_SOURCE_DIR}/release-1.11.0.tar.gz
# PROJECT_BINARY_DIR : 就是 cmake -B 参数指定的目录
# CMAKE_SOURCE_DIR : cmake的顶层目录
execute_process(COMMAND 
    ${CMAKE_COMMAND} -E 
    tar xf ${CMAKE_SOURCE_DIR}/release-1.11.0.tar.gz
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# 配置
# cmake -S ${PROJECT_BINARY_DIR}/googletest-release-1.11.0 -B ${PROJECT_BINARY_DIR}/googletest-release-1.11.0/build
set(GTEST_SOURCE ${PROJECT_BINARY_DIR}/googletest-release-1.11.0)
execute_process(COMMAND
    ${CMAKE_COMMAND} -S ${GTEST_SOURCE} -B ${GTEST_SOURCE}/build)

# 编译
execute_process(COMMAND
    ${CMAKE_COMMAND} --build ${GTEST_SOURCE}/build)

# 安装
execute_process(COMMAND
    ${CMAKE_COMMAND} --install ${GTEST_SOURCE}/build --prefix=${GTEST_PATH})
```

完整示例
```bash

cmake_minimum_required(VERSION 3.22)
project(gtest-test)

set(GTEST_PATH ${CMAKE_SOURCE_DIR}/gtest)

# 如果没有安装GTest，则安装
if (NOT EXISTS ${GTEST_PATH})
# 解压
# cd ${PROJECT_BINARY_DIR}
# cmake -E tar xf ${CMAKE_SOURCE_DIR}/release-1.11.0.tar.gz
# PROJECT_BINARY_DIR : 就是 cmake -B 参数指定的目录
# CMAKE_SOURCE_DIR : cmake的顶层目录
    execute_process(COMMAND 
        ${CMAKE_COMMAND} -E 
        tar xf ${CMAKE_SOURCE_DIR}/release-1.11.0.tar.gz
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR})


# 配置
# cmake -S ${PROJECT_BINARY_DIR}/googletest-release-1.11.0 -B ${PROJECT_BINARY_DIR}/googletest-release-1.11.0/build
    set(GTEST_SOURCE ${PROJECT_BINARY_DIR}/googletest-release-1.11.0)
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

add_executable(main main.cc)
# 设置CMAKE_PREFIX_PATH ，否则找不到GTest的config文件
set(CMAKE_PREFIX_PATH ${GTEST_PATH}/lib/cmake)
find_package(GTest)
target_link_libraries(main GTest::gtest_main)

# 将gtest测试程序main，将他包装成ctest
include(GoogleTest)
gtest_discover_tests(main)

# 启动测试
enable_testing()
```

