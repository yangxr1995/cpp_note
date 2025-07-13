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



