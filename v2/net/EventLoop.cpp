#include "EventLoop.h"

#include <functional>
#include <iostream>
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
    std::cout << "threadID " << m_threadID << std::endl;

    std::vector<IEventDispatcher*> eventDispatchers;

    while (m_running) {
        //1. 检测和处理定时器事件
        checkAndDoTimers();

        // 
        //2. 使用select/poll/epoll等IO复用函数检测一组socket的读写事件
        // 
        eventDispatchers.clear();
        m_spIOMultiplex->poll(500000, eventDispatchers);

        //3. 处理读写事件
        for (size_t i = 0; i < eventDispatchers.size(); ++i) {
            eventDispatchers[i]->onRead();
            eventDispatchers[i]->onWrite();
        }

        //4. 利用唤醒fd机制处理自定义事件
        doOtherTasks();
    }
}

void EventLoop::addTask(const CustomTask& task) {
    {
        std::lock_guard<std::mutex> scopedLock(m_mutexTasks);
        m_customTasks.push_back(task);
    }

    m_pWakeupEventDispatcher->wakeup();
}

void EventLoop::setThreadID(const std::thread::id& threadID) {
    m_threadID = threadID;
}

const std::thread::id& EventLoop::getThreadID() const {
    return m_threadID;
}

bool EventLoop::isCallableInOwnerThread() const {
    return std::this_thread::get_id() == m_threadID;
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

int64_t EventLoop::addTimer(int32_t intervalMs, bool repeated, int64_t repeatedCount, TimerTask timerTask,
    TimerMode mode/* = TimerMode::TimerModeFixedInterval*/) {
    //对传入的参数进行校验

    auto spTimer = std::make_shared<Timer>(intervalMs,
        repeated, repeatedCount, timerTask, mode);

    addTimerInternal(spTimer);

    return spTimer->id();
}

void EventLoop::removeTimer(int64_t timerID) {
    removeTimerInternal(timerID);
}

bool EventLoop::createWakeupfd() {
    m_wakeupfd = ::eventfd(0, EFD_NONBLOCK);
    if (m_wakeupfd == -1)
        return false;

    return true;
}

void EventLoop::checkAndDoTimers() {
    //if (!m_isBaseLoop) {
    //    return;
    //}

    int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();;

    m_isCheckTimers = true;
    //std::lock_guard<decltype(m_mutexTimers)> scopedLock(m_mutexTimers);
    for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++) {
        while (nowMs >= (*iter)->nextTriggeredTimeMs()) {
            //std::cout << "checkAndDoTimers, timerID " << (*iter)->id()
            //    << " threadID " << std::this_thread::get_id() << std::endl;

            (*iter)->doTimer((*iter)->id(), nowMs);
        }
    }

    m_isCheckTimers = false;

    if (!m_pendingRemoveTimers.empty()) {
        for (auto& timerID : m_pendingRemoveTimers)
            removeTimerInternal(timerID);
    }

    if (!m_pendingAddTimers.empty()) {
        for (auto& spTimer : m_pendingAddTimers)
            addTimerInternal(spTimer);
    }
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

void EventLoop::addTimerInternal(std::shared_ptr<Timer> spTimer) {
    if (isCallableInOwnerThread()) {
        if (m_isCheckTimers) {
            m_pendingAddTimers.push_back(spTimer);
        } else {
            std::cout << "addTimerInternal, timerID " << spTimer->id()
                << " threadID " << std::this_thread::get_id() << std::endl;

            m_timers.push_back(spTimer);
        }
    } else {
        addTask(std::bind(&EventLoop::addTimerInternal, this, spTimer));
    }
}

void EventLoop::removeTimerInternal(int64_t timerID) {
    if (isCallableInOwnerThread()) {
        if (m_isCheckTimers) {
            m_pendingRemoveTimers.push_back(timerID);
        } else {
            for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++) {
                if ((*iter)->id() == timerID) {

                    std::cout << "removeTimerInternal, timerID " << timerID
                        << " threadID " << std::this_thread::get_id() << std::endl;

                    m_timers.erase(iter);
                    return;
                }
            }
        }
    } else {
        addTask(std::bind(&EventLoop::removeTimerInternal, this, timerID));
    }
}