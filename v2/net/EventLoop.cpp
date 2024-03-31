#include "EventLoop.h"

#include <memory>
#include <vector>

#include <sys/eventfd.h>

#include "Select.h"
#include "Poll.h"
#include "Epoll.h"

EventLoop::~EventLoop() {
    if (m_pWakeupEventDispatcher)
        delete m_pWakeupEventDispatcher;
}

bool EventLoop::init(IOMultiplexType type/* = IOMultiplexType::IOMultiplexEpoll*/) {
    if (type == IOMultiplexType::IOMultiplexTypeSelect) {
        m_spIOMultiplex = std::make_unique<Select>();
    } else if (type == IOMultiplexType::IOMultiplexTypePoll) {
        m_spIOMultiplex = std::make_unique<Poll>();
    } else {
        m_spIOMultiplex = std::make_unique<Epoll>();
    }

    if (!createWakeupfd())
        return false;

    m_pWakeupEventDispatcher = new WakeupEventDispatcher(m_wakeupfd);

    registerReadEvent(m_wakeupfd, m_pWakeupEventDispatcher, true);

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
        doOtherTasks();
    }
}

void EventLoop::addTask(CustomTask&& task) {
    {
        std::lock_guard<std::mutex> scopedLock(m_mutexTasks);
        m_customTasks.push_back(std::move(task));
    }

    m_pWakeupEventDispatcher->wakeup();
}

void EventLoop::setThreadID(const std::thread::id& threadID) {
    m_threadID = threadID;
}

const std::thread::id& EventLoop::getThreadID() const {
    return m_threadID;
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

bool EventLoop::createWakeupfd() {
    m_wakeupfd = ::eventfd(0, EFD_NONBLOCK);
    if (m_wakeupfd == -1)
        return false;

    return true;
}

void EventLoop::doOtherTasks() {
    std::vector<CustomTask> tasks;

    {
        std::lock_guard<std::mutex> scopedLock(m_mutexTasks);
        tasks.swap(m_customTasks);
    }

    for (auto& task : tasks) {
        task();
    }
}