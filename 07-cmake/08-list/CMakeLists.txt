cmake_minimum_required(VERSION 3.20)
project(list-test)

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


