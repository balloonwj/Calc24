#include "TCPConnection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <iostream>

TCPConnection::TCPConnection(int clientfd, const std::shared_ptr<EventLoop>& spEventLoop)
    : m_fd(clientfd), m_spEventLoop(spEventLoop) {
    std::cout << "TCPConnection::ctor " << m_fd << std::endl;
}

TCPConnection::~TCPConnection() {
    std::cout << "TCPConnection::dtor " << m_fd << std::endl;
    ::close(m_fd);
}

bool TCPConnection::startRead() {
    m_spEventLoop->registerReadEvent(m_fd, this, true);

    return true;
}

bool TCPConnection::send(const char* buf, int bufLen) {
    m_sendBuf.append(buf, bufLen);

    while (true) {
        int n = ::send(m_fd, m_sendBuf, m_sendBuf.remaining(), 0);
        if (n == 0) {
            //对端关闭了连接
            //TODO: 我们也关闭连接
            onClose();
            return false;
        } else if (n < 0) {
            if (errno == EINTR) {
                continue;;
            } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                //当前由于TCP窗口太小，数据发不出去了
                m_writeCallback();

                registerWriteEvent();

                return true;
            }

            //其他情况出错了，关闭连接
            onClose();
            return false;
        }

        //发送成功了
        m_sendBuf.erase(n);
        if (m_sendBuf.isEmpty()) {
            return true;
        }
    }

    //return false;
}


bool TCPConnection::send(const std::string& buf) {
    //当前调用线程和当前TCPConnection属于同一个线程，则直接发送；
    //反之，交给TCPConnection属于同所属的线程发送
    if (isCallableInOwnerThread()) {
        return send(buf.c_str(), buf.length());
    } else {
        m_spEventLoop->addTask(std::bind(&TCPConnection::send, this, buf));
    }
}

void TCPConnection::onRead() {
    if (!m_enableRead)
        return;

    //收数据
    char buf[1024];
    int n = ::recv(m_fd, buf, sizeof(buf), 0);
    if (n == 0) {
        //对端关闭了连接
        //我们也关闭连接
        onClose();
    } else if (n < 0) {
        if (errno == EINTR/* || errno == EWOULDBLOCK || errno == EAGAIN*/) {
            return;
        }

        //其他情况出错了，关闭连接
        onClose();
        return;
    }

    //正常收到数据包
    m_recvBuf.append(buf, n);

    //解包
    //m_readCallback=>Calc24Session::onRead
    m_readCallback(m_recvBuf);
}

void TCPConnection::onWrite() {
    if (!m_enableWrite)
        return;

    while (true) {
        int n = ::send(m_fd, m_sendBuf, m_sendBuf.remaining(), 0);
        if (n == 0) {
            //对端关闭了连接
            //TODO: 我们也关闭连接
            return;
        } else if (n < 0) {
            if (errno == EINTR) {
                continue;;
            } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                //当前由于TCP窗口太小，数据发不出去了
                m_writeCallback();

                unregisterWriteEvent();

                return;
            }

            //其他情况出错了，关闭连接
            return;
        }

        //发送成功了
        m_sendBuf.erase(n);
        if (m_sendBuf.isEmpty()) {
            return;
        }
    }
}

void TCPConnection::onClose() {
    std::cout << "TCPConnection::onClose, " << m_fd << std::endl;

    unregisterAllEvents();

    //Calc24Session::onClose -> Calc24Server::onDisconnected -> 从map中移除Calc24Session对象
    //->析构Calc24Session->先析构TCPConnection->关闭fd
    m_closeCallback(shared_from_this());
}

void TCPConnection::enableReadWrite(bool read, bool write) {
    m_enableRead = read;
    m_enableWrite = write;
}

void TCPConnection::registerWriteEvent() {
    if (m_registerWriteEvent)
        return;

    //向IO复用函数注册写事件
    m_spEventLoop->registerWriteEvent(m_fd, this, true);
}

void TCPConnection::unregisterWriteEvent() {
    if (!m_registerWriteEvent)
        return;

    //向IO复用函数反注册写事件
    m_spEventLoop->unregisterWriteEvent(m_fd, this, false);

    m_registerWriteEvent = false;
}

void TCPConnection::unregisterAllEvents() {
    //向IO复用函数反注册所有读写事件
    m_spEventLoop->unregisterAllEvents(m_fd, this);

    m_registerWriteEvent = false;
}

bool TCPConnection::isCallableInOwnerThread() {
    return std::this_thread::get_id() == m_spEventLoop->getThreadID();
}