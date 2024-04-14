#pragma once

#include "ThreadPool.h"

#include <map>
#include <string>

#include "Acceptor.h"
#include "EventLoop.h"
#include "TCPConnection.h"

using onConnectedCallback = std::function<void(std::shared_ptr<TCPConnection>& spConn)>;
using onDisconnectedCallback = std::function<void(const std::shared_ptr<TCPConnection>& spConn)>;

class TCPServer {
public:
    TCPServer();
    virtual ~TCPServer() = default;

    bool init(int32_t threadNum, const std::string& ip, uint16_t port);
    void uninit();

    void start();

    //m_connectedCallback需要设置为指向业务的连接回调，
    //以让业务在有新连接来临时做一些操作的机会
    void setConnectedCallback(onConnectedCallback&& callback) {
        m_connectedCallback = std::move(callback);
    }

    void setDisconnectedCallback(onDisconnectedCallback&& callback) {
        m_disconnectedCallback = std::move(callback);
    }

    EventLoop& getBaseEventLoop() {
        return m_baseEventLoop;
    }

private:
    void onAccept(int clientfd);
    void onDisconnected(const std::shared_ptr<TCPConnection>& spConn);


private:
    ThreadPool                                      m_threadPool;

    int                                             m_listenfd{ -1 };
    std::string                                     m_ip;
    uint16_t                                        m_port;

    EventLoop                                       m_baseEventLoop;
    Acceptor* m_pAcceptor;

    std::map<int, std::shared_ptr<TCPConnection>>   m_connections;

    onConnectedCallback                             m_connectedCallback;
    onDisconnectedCallback                          m_disconnectedCallback;
};
