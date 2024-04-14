#pragma once

#include "TCPServer.h"

#include <string>

#include "Calc24Session.h"

class Calc24Server final {
public:
    Calc24Server() = default;
    ~Calc24Server() = default;

    bool init(int32_t threadNum, const std::string& ip = "", uint16_t port = 8888);
    void uninit();

    void sendAll(const std::string& msg, bool includeSelf, int32_t id);

    void onDisconnected(int32_t id);

private:
    void onConnected(std::shared_ptr<TCPConnection>& spConn);


private:
    TCPServer                                           m_tcpServer;
    //id=>std::shared_ptr<Calc24Session>
    std::map<int32_t, std::shared_ptr<Calc24Session>>   m_sessions;
    std::vector<std::shared_ptr<Calc24Session>>         m_pendingToDeleteSessions;

    int64_t                                             m_checkHandupTimerID{ 0 };
};