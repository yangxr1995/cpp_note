cmake_minimum_required(VERSION 3.22)
project(install-doc)

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

# 安装指定目录下所有文件，包含子目录
# 指定文件类型，安装到相关类型的默认目录
# install(DIRECTORY doc TYPE DOC)
# # 指定安装目录
# install(DIRECTORY doc DESTINATION doc2)

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

