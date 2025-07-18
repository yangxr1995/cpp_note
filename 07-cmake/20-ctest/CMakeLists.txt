cmake_minimum_required(VERSION 3.22)
project(test_ctest)

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

add_test(NAME test_ok
    COMMAND main aaa)
set_tests_properties(test_ok
    PROPERTIES
    PASS_REGULAR_EXPRESSION aaa
    FAIL_REGULAR_EXPRESSION bbb)

add_test(NAME test_fail
    COMMAND main bbb)
set_tests_properties(test_fail
    PROPERTIES
    PASS_REGULAR_EXPRESSION aaa
    FAIL_REGULAR_EXPRESSION bbb)

enable_testing()
