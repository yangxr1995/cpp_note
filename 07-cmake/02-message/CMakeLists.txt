cmake_minimum_required(VERSION 3.20)
project(message-test)

# 行注释

#[[
 块注释
]]

message(CHECK_START "查找xlog")
message(CHECK_START "查找pthread")
message(CHECK_START "查找rt")
message(CHECK_PASS "Found")
message(CHECK_PASS "Found")
message(CHECK_FAIL "Not found")
message(STATUS "--------")


set(VAR1 "VAR1 init" CACHE STRING "var1 cache var")
message("VAR1 : ${VAR1}") # VAR1 init
set(VAR1 "VAR1 modify" CACHE STRING "var1 cache var")
message("VAR1 : ${VAR1}") # VAR1 init
set(VAR1 "VAR1 modify" CACHE STRING "var1 cache var" FORCE)
message("VAR1 : ${VAR1}") # VAR1 modify 


