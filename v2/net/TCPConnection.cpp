#include "TCPConnection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

TCPConnection::TCPConnection(int clientfd, const std::shared_ptr<EventLoop>& spEventLoop)
    : m_fd(clientfd), m_spEventLoop(spEventLoop) {

}

TCPConnection::~TCPConnection() {
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
    return send(buf.c_str(), buf.length());
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
    m_closeCallback();

    unregisterAllEvents();
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