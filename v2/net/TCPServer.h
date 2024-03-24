#pragma once

#include "ThreadPool.h"

#include <map>
#include <string>

#include "Acceptor.h"
#include "EventLoop.h"
#include "TCPConnection.h"

using onConnectedCallback = std::function<void(std::shared_ptr<TCPConnection>& spConn)>;
using onDisconnectedCallback = std::function<void(std::shared_ptr<TCPConnection>& spConn)>;

class TCPServer {
public:
    TCPServer() = default;
    virtual ~TCPServer() = default;

    bool init(int32_t threadNum, const std::string& ip, uint16_t port);
    void uninit();

    void start();

    void setConnectedCallback(onConnectedCallback&& callback) {
        m_connectedCallback = std::move(callback);
    }

    void setDisconnectedCallback(onDisconnectedCallback&& callback) {
        m_disconnectedCallback = std::move(callback);
    }

private:
    void onAccept(int clientfd);


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
