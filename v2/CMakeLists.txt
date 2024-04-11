cmake_minimum_required(VERSION 3.20)

project (CALC24)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -g -Wall -O0 -Wno-unused-variable -pthread")

include_directories(
    utils/
	net/
)

link_directories(
    /lib64
)

set(net_srcs 
net/Acceptor.cpp
net/ByteBuffer.cpp
net/Epoll.cpp
net/EventLoop.cpp
net/Poll.cpp
net/Select.cpp
net/TCPConnection.cpp
net/TCPServer.cpp
net/ThreadPool.cpp
net/WakeupEventDispatcher.cpp
net/Timer.cpp
)

set(util_srcs
utils/util.cpp
)

set(app_srcs
appsrc/Calc24Server.cpp
appsrc/Calc24Session.cpp
appsrc/main.cpp
)


add_executable(calc24Srv ${net_srcs} ${util_srcs} ${app_srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
TARGET_LINK_LIBRARIES(calc24Srv)
