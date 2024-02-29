#pragma once

#include <string>

class TCPServer
{
public:
    TCPServer() = default;
    ~TCPServer() = default;

    TCPServer(const TCPServer& rhs) = delete;
    TCPServer& operator=(const TCPServer& rhs) = delete;

    //TODO: 移动拷贝构造函数
    //TODO: 移动operator =

    bool init(const std::string& ip, uint16_t port);
};
