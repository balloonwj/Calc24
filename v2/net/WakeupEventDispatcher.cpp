#include "WakeupEventDispatcher.h"

#include <errno.h>
#include <string.h>
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
    uint64_t dummyData;
    int n = ::read(m_wakeupfd, static_cast<void*>(&dummyData), sizeof(dummyData));
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
    uint64_t dummyData = 0;
    //对于eventfd，至少要写入8个字节
    int n = ::write(m_wakeupfd, static_cast<const void*>(&dummyData), static_cast<size_t>(sizeof(dummyData)));
    if (n != sizeof(dummyData)) {
        std::cout << "WakeupEventDispatcher::wakeup failed, errno " << errno << " " << strerror(errno) << std::endl;
    } else {
        std::cout << "WakeupEventDispatcher::wakeup successfully, m_wakeupfd " << m_wakeupfd << std::endl;
    }
}