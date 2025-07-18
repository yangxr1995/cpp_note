cmake_minimum_required(VERSION 3.20)

project(var-test)

set(VAR1 "aa")
set(VAR2 "VAR1")
message(STATUS "VAR1 = ${VAR1}")     # VAR1 = aa
message(STATUS "VAR2 = ${VAR2}")     # VAR2 = VAR1
message(STATUS "$\{VAR2\} = ${${VAR2}}")  # ${VAR2} = aa
unset(VAR1)
message(STATUS "VAR1 = ${VAR1}")     # VAR1 = 
