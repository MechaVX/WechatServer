cmake_minimum_required(VERSION 2.8.3)

project(WechatServer)

include_directories(
${CMAKE_SOURCE_DIR}/inc 
${CMAKE_SOURCE_DIR}/inc/tcpserver
${CMAKE_SOURCE_DIR}/inc/mysql
${CMAKE_SOURCE_DIR}/inc/messageworkers
${CMAKE_SOURCE_DIR}/inc/files
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -std=c++11")

#获取指定目录下所有文件，赋值于变量SRCX
aux_source_directory(./src/tcpserver SRC1)
aux_source_directory(./src/mysql SRC2)
aux_source_directory(./src/messageworkers SRC3)
aux_source_directory(./src/files SRC4)
set(SRC ${SRC1} ${SRC2} ${SRC3} ${SRC4})
#set(SRCS main.cc ${SRC})
#message("Create executable file: build/server_exe")
#add_executable(server_exe ${SRCS})
#target_link_libraries(server_exe pthread)

#set(SRCS_SELECT main_select.cc ${SRC})
#message("Create executable file: build/server_select_exe")
#add_executable(server_select_exe ${SRCS_SELECT})
#target_link_libraries(server_select_exe pthread)

set(SRCS_EPOLL main_epoll.cc ${SRC})
message("Create executable file: build/server_epoll_exe")
add_executable(server_epoll_exe ${SRCS_EPOLL})
target_link_libraries(server_epoll_exe pthread mysqlclient)

add_executable(mysql_example_exe main_mysql.cc)
target_link_libraries(mysql_example_exe pthread mysqlclient)

#gdb调试程序
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -std=gnu++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG -Wall -O0 -std=c++11")
message("Create gdb executable file: build/gdb_server_epoll_exe")
add_executable(gdb_server_epoll_exe ${SRCS_EPOLL})
target_link_libraries(gdb_server_epoll_exe pthread mysqlclient)

