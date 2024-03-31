#include "Calc24Server.h"

#include <iostream>

#include "Calc24Session.h"

bool Calc24Server::init(int32_t threadNum, const std::string& ip/* = ""*/, uint16_t port/* = 8888*/) {
    //m_tcpServer.setConnectedCallback([](std::shared_ptr<TCPConnection>& spConn) -> void {

    //    });

    m_tcpServer.setConnectedCallback(std::bind(&Calc24Server::onConnected, this, std::placeholders::_1));
    //m_tcpServer.setDisconnectedCallback(std::bind(&Calc24Server::onDisconnected, this, std::placeholders::_1));

    if (!m_tcpServer.init(5, ip, port)) {
        return false;
    }

    m_tcpServer.start();

    return true;
}

void Calc24Server::uninit() {
    m_tcpServer.uninit();
}

void Calc24Server::sendAll(const std::string& msg, bool includeSelf, int32_t id) {
    for (const auto& iter : m_sessions) {
        if (!includeSelf) {
            if (iter.second->getID() == id) {
                continue;
            }
        }
        //iter指向的std::pair<int32_t, std::shared_ptr<Calc24Session>>
        //iter.second指向std::shared_ptr<Calc24Session>
        iter.second->sendMsg(msg);
    }
}

void Calc24Server::onConnected(std::shared_ptr<TCPConnection>& spConn) {
    //删除无效的Calc24Session
    m_pendingToDeleteSessions.clear();

    auto spCalc24Session = std::make_shared<Calc24Session>(this, std::move(spConn));
    spCalc24Session->sendWelcomeMsg();

    //std::pair<int, std::shared_ptr<Calc24Session>>
    m_sessions.emplace(spCalc24Session->getID(), std::move(spCalc24Session));
}

void Calc24Server::onDisconnected(int32_t id) {
    auto iter = m_sessions.find(id);
    if (iter != m_sessions.end()) {
        auto pendingToDeleteSession = iter->second;
        m_pendingToDeleteSessions.push_back(std::move(pendingToDeleteSession));

        m_sessions.erase(iter);

        std::cout << "Calc24Server::onDisconnected" << std::endl;
    }
}