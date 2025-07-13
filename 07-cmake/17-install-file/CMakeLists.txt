cmake_minimum_required(VERSION 3.22)
project(install-file)

file(WRITE a.h "")
file(WRITE b.h "")
file(WRITE c.h "")
file(WRITE d.h "")

# 将文件安装到指定目录
install(FILES a.h b.h DESTINATION include)

# 添加OPTIONAL : 文件可以不存在
install(FILES e.h  DESTINATION include OPTIONAL)


# 根据GNU规则将文件按照类型，安装到约定目录
include(GNUInstallDirs)
message("CMAKE_INSTALL_DATAROOTDIR : ${CMAKE_INSTALL_DATAROOTDIR}")
install(FILES a.h TYPE DOC)
install(FILES b.h TYPE LIB)
install(FILES c.h TYPE INCLUDE)

# 文件权限
install(FILES a.h DESTINATION pub
    PERMISSIONS 
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_EXECUTE
    WORLD_READ WORLD_WRITE WORLD_EXECUTE)
