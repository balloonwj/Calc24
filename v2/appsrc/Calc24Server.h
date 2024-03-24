#pragma once

#include "TCPServer.h"

#include "Calc24Session.h"

class Calc24Server final {
public:
    Calc24Server() = default;
    ~Calc24Server() = default;

    bool init(int32_t threadNum, const std::string& ip = "", uint16_t port = 8888);
    void uninit();

private:
    void onConnected(std::shared_ptr<TCPConnection>& spConn);
    void onDisconnected(std::shared_ptr<TCPConnection>& spConn);

private:
    TCPServer                                       m_tcpServer;

    std::vector<std::shared_ptr<Calc24Session>>     m_sessions;
};