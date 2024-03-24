#include "Calc24Server.h"

bool Calc24Server::init(int32_t threadNum, const std::string& ip/* = ""*/, uint16_t port/* = 8888*/) {
    //m_tcpServer.setConnectedCallback([](std::shared_ptr<TCPConnection>& spConn) -> void {

    //    });

    m_tcpServer.setConnectedCallback(std::bind(&Calc24Server::onConnected, this, std::placeholders::_1));

    if (!m_tcpServer.init(5, ip, port)) {
        return false;
    }

    m_tcpServer.start();

    return true;
}

void Calc24Server::uninit() {
    m_tcpServer.uninit();
}

void Calc24Server::onConnected(std::shared_ptr<TCPConnection>& spConn) {
    auto spCalc24Session = std::make_shared<Calc24Session>(std::move(spConn));
    spCalc24Session->sendWelcomeMsg();

    m_sessions.push_back(std::move(spCalc24Session));
}

void Calc24Server::onDisconnected(std::shared_ptr<TCPConnection>& spConn) {

}