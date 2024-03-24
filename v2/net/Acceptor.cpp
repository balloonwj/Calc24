#include "Acceptor.h"

#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


Acceptor::Acceptor(EventLoop* pEventLoop) : m_pEventLoop(pEventLoop) {

}

Acceptor::~Acceptor() {
    if (m_listenfd != -1) {
        ::close(m_listenfd);
    }
}

void Acceptor::onRead() {
    while (true) {
        struct sockaddr clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        //4. 接受客户端连接
        int clientfd = ::accept4(m_listenfd, (struct sockaddr*)&clientAddr, &clientAddrLen, SOCK_NONBLOCK);
        if (clientfd > 0) {
            //成功接收连接
            //m_acceptCallback=>TCPServer::onAccept
            m_acceptCallback(clientfd);
        } else if (clientfd == -1) {
            if (errno == EWOULDBLOCK) {
                //没有连接了
                return;
            } else {
                //出错了
                return;
            }
        }
    }

}

bool Acceptor::startListen(const std::string& ip/* = ""*/, uint16_t port/* = 8888*/) {
    //1.创建一个侦听socket
    m_listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd == -1)
        return false;

    int optval = 1;
    //TODO: 判断一下这两个函数是否调用成功
    ::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    ::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));

    //2.初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    //TODO: 待修复
    if (ip == "") {
        bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        bindaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    bindaddr.sin_port = htons(port);
    if (::bind(m_listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1) {
        std::cout << "bind listen socket error." << std::endl;
        return false;
    }

    //3.启动侦听
    if (::listen(m_listenfd, SOMAXCONN) == -1) {
        std::cout << "listen error." << std::endl;
        return false;
    }

    m_pEventLoop->registerReadEvent(m_listenfd, this, true);

    return true;
}