/**
 * zhangxf 2024.03.14
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "IEventDispatcher.h"
#include "IOMultiplex.h"
#include "WakeupEventDispatcher.h"

using CustomTask = std::function<bool(const std::string&)>;

enum class IOMultiplexType {
    IOMultiplexTypeSelect,
    IOMultiplexTypePoll,
    IOMultiplexEpoll
};

class EventLoop final {
public:
    EventLoop() = default;
    ~EventLoop();

public:
    bool init(IOMultiplexType type = IOMultiplexType::IOMultiplexEpoll);

    void run();

    void addTask(CustomTask&& task);

    void setThreadID(const std::thread::id& threadID);
    const std::thread::id& getThreadID() const;

    void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
    void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
    void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
    void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
    void unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher);

private:
    bool createWakeupfd();

    void doOtherTasks();

private:
    bool                                    m_running{ false };
    int                                     m_epollfd;
    int                                     m_wakeupfd;

    std::unique_ptr<IIOMultiplex>           m_spIOMultiplex;
    std::thread::id                         m_threadID;

    WakeupEventDispatcher* m_pWakeupEventDispatcher{ nullptr };

    std::vector<CustomTask>                 m_customTasks;
    std::mutex                              m_mutexTasks;
};
