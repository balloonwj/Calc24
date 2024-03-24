#include "Poll.h"

//void Poll::poll(int timeoutUs, std::vector<IEventDispatcher*> triggeredEventDispatchers) {
//
//    //使用poll函数
//    int n = poll(....);
//
//    //拿到有事件的fd
//    //遍历有事件的fd，通过m_eventDispatchers取到IEventDispatcher*对象
//    triggeredEventDispatchers.push_back(eventDispatcher);
//}
//
//void Poll::registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) {
//    //往数组m_pollfd后面添加元素
//    //向m_pollfd结构中添加pollfd
//    m_eventDispatchers[fd] = eventDispatcher;
//}
//
//
//void Poll::registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) {
//    //往数组m_pollfd后面添加元素
//    //往数组m_pollfd后面添加元素
//    //向m_pollfd结构中添加pollfd
//    m_eventDispatchers[fd] = eventDispatcher;
//}