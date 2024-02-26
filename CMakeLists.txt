#cmake版本
cmake_minimum_required(VERSION 3.10.2)

#工程名称
project(sipServer VERSION 0.1.0 LANGUAGES C CXX)

#设置显性变量
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ")
#set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE "Debug")
#O0-O4表示代码没优化-最大优化
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#编译期间打印消息
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")

set(INCLUDE_DIR /usr/local/include)
set(INCLUDE_DIR2 ./utils)
set(INCLUDE_DIR3 /usr/include)
set(INCLUDE_DIR4 /usr/local/include/orocos)
set(INCLUDE_DIR5 /usr/include/x86_64-linux-gnu)

set(LIB_DIR /usr/local/lib)
set(LIB_DIR2 /usr/lib/x86_64-linux-gnu)
#头文件目录包含
include_directories(${INCLUDE_DIR})
include_directories(${INCLUDE_DIR2})
include_directories(${INCLUDE_DIR3})
include_directories(${INCLUDE_DIR4})
include_directories(${INCLUDE_DIR5})
#链接库目录包含
link_directories(${LIB_DIR})
link_directories(${LIB_DIR2})

include(CTest)
enable_testing()

#生成可执行文件
add_executable(sipServer 
utils/MD5.c
utils/HTTPDigest.c
utils/func.cpp
utils/mylog.cpp 
utils/myconfig.cpp
SipServer.cpp
main.cpp)

#curl主库 ssl和crypto是openssl里的
TARGET_LINK_LIBRARIES(sipServer curl ssl crypto osip2 osipparser2 eXosip2 mysqlclient pthread websockets orocos-log4cpp jsoncpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
