cmake_minimum_required(VERSION 3.22)
project(cross-compile-test)

file(WRITE main.cc [=[
    int main() {
    return 0;
    }
    ]=])

add_executable(main main.cc)


