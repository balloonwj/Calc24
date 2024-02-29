#pragma once

#include <string>

class TCPClient
{
public:
    TCPClient() = default;
    ~TCPClient() = default;

    //TODO: 拷贝构造函数
    //TODO: operator=
    //TODO: 移动构造
    //TODO: 移动赋值

public:
    bool init(const std::string& serverIP, uint16_t serverPort);
};
