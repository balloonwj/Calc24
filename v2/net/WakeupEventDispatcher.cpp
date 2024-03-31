#include "WakeupEventDispatcher.h"

#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

WakeupEventDispatcher::WakeupEventDispatcher(int fd) : m_wakeupfd(fd) {

}

WakeupEventDispatcher::~WakeupEventDispatcher() {
    ::close(m_wakeupfd);
}

void WakeupEventDispatcher::onRead() {
    if (!m_enableRead)
        return;

    //收数据
    char dummyData;
    int n = ::recv(m_wakeupfd, static_cast<char*>(&dummyData), sizeof(dummyData), 0);
    if (n != sizeof(dummyData)) {
        std::cout << "WakeupEventDispatcher::onRead failed, errno " << errno << std::endl;
    } else {
        std::cout << "WakeupEventDispatcher::onRead successfully, m_wakeupfd " << m_wakeupfd << std::endl;
    }
}

void WakeupEventDispatcher::enableReadWrite(bool read, bool write) {
    m_enableRead = read;
    m_enableWrite = write;
}

void WakeupEventDispatcher::wakeup() {
    char dummyData = 0;
    int n = ::send(m_wakeupfd, static_cast<const void*>(&dummyData), sizeof(dummyData), 0);
    if (n != sizeof(dummyData)) {
        std::cout << "WakeupEventDispatcher::wakeup failed, errno " << errno << std::endl;
    } else {
        std::cout << "WakeupEventDispatcher::wakeup successfullt, m_wakeupfd " << m_wakeupfd << std::endl;
    }
}