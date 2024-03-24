#include "TCPServer.h"

#include <functional>

bool TCPServer::init(int32_t threadNum, const std::string& ip, uint16_t port) {
    m_threadPool.start(threadNum);

    //TODO：对ip和port做一些必要的参数校验
    m_ip = ip;
    m_port = port;

    m_baseEventLoop.init();

    m_pAcceptor = new Acceptor(&m_baseEventLoop);
    if (!m_pAcceptor->startListen(ip, port)) {
        return false;
    }

    //TODO: AcceptCallback只接收一个参数，这里传了两个，会有问题？
    m_pAcceptor->setAcceptCallback(std::bind(&TCPServer::onAccept, this, std::placeholders::_1));

    return true;
}

void TCPServer::uninit() {
    m_threadPool.stop();
}

void TCPServer::start() {
    m_baseEventLoop.run();
}

void TCPServer::onAccept(int clientfd) {
    std::shared_ptr<EventLoop> spEventLoop = m_threadPool.getNextEventLoop();
    auto spTCPConnection = std::make_shared<TCPConnection>(clientfd, spEventLoop);
    spTCPConnection->startRead();

    m_connectedCallback(spTCPConnection);

    //m_connections[clientfd] = std::move(spTCPConnection);
}

