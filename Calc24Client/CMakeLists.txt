cmake_minimum_required(VERSION 2.6)

project (CALC24_CLIENT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -O0 -Wno-unused-variable -pthread")

#link_directories(
#)


set(srcs 
TCPClient.cpp
Calc24Client.cpp
)

add_executable(calc24Client ${srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
TARGET_LINK_LIBRARIES(calc24Client)