#include "EventLoop.h"

#include <memory>
#include <vector>

#include "Select.h"
#include "Poll.h"
#include "Epoll.h"

bool EventLoop::init(IOMultiplexType type/* = IOMultiplexType::IOMultiplexEpoll*/) {
    if (type == IOMultiplexType::IOMultiplexTypeSelect) {
        m_spIOMultiplex = std::make_unique<Select>();
    } else if (type == IOMultiplexType::IOMultiplexTypePoll) {
        m_spIOMultiplex = std::make_unique<Poll>();
    } else {
        m_spIOMultiplex = std::make_unique<Epoll>();
    }

    m_running = true;

    return true;
}

void EventLoop::run() {
    std::vector<IEventDispatcher*> eventDispatchers;

    while (m_running) {
        //1. 检测和处理定时器事件
        // 
        //2. 使用select/poll/epoll等IO复用函数检测一组socket的读写事件
        // 
        eventDispatchers.clear();
        m_spIOMultiplex->poll(500000, eventDispatchers);
        for (size_t i = 0; i < eventDispatchers.size(); ++i) {
            eventDispatchers[i]->onRead();
            eventDispatchers[i]->onWrite();
        }
        //3. 处理读写事件
        //for ()

        //4. 利用唤醒fd机制处理自定义事件 
    }
}

void EventLoop::registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) {
    m_spIOMultiplex->registerReadEvent(fd, eventDispatcher, readEvent);
}

void EventLoop::registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) {
    m_spIOMultiplex->registerWriteEvent(fd, eventDispatcher, writeEvent);
}

void EventLoop::unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) {
    m_spIOMultiplex->unregisterReadEvent(fd, eventDispatcher, readEvent);
}

void EventLoop::unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) {
    m_spIOMultiplex->unregisterWriteEvent(fd, eventDispatcher, writeEvent);
}

void EventLoop::unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher) {
    m_spIOMultiplex->unregisterAllEvents(fd, eventDispatcher);
}