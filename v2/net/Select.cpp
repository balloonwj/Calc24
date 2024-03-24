#include "Select.h"

//void Select::poll(int timeoutUs, std::vector<IEventDispatcher*> triggeredEventDispatchers) {
//    struct timeval tv;
//    tv.tv_sec = timeoutUs / 1000000;
//    tv.tv_usec = timeoutUs - timeoutUs / 1000000 * 1000000;
//
//
//    int n = ::select(m_maxfd + 1, &m_readfds, &m_writefds, nullptr, &tv);
//    if (n <= 0) {
//        return;
//    }
//
//    //TODO: 遍历m_readfds，调用FD_ISSET判断哪些socket可读
//    //fd => IEventDispatcher
//    triggeredEventDispatchers.push_back(fd1);
//
//    //TODO: 遍历m_writefds，调用FD_ISSET判断哪些socket可写
//
//}
//
//void Select::registerReadEvent(int fd, bool readEvent) {
//    if (m_maxfd < fd) {
//        m_maxfd = fd;
//    }
//
//    FD_SET(fd, &m_readfds);
//}
//
//void Select::registerWriteEvent(int fd, bool writeEvent) {
//    if (m_maxfd < fd) {
//        m_maxfd = fd;
//    }
//
//    FD_SET(fd, &m_writefds);
//}