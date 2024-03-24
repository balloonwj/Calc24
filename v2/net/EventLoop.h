/**
 * zhangxf 2024.03.14
 */

#pragma once

#include <memory>

#include "IEventDispatcher.h"
#include "IOMultiplex.h"

enum class IOMultiplexType {
    IOMultiplexTypeSelect,
    IOMultiplexTypePoll,
    IOMultiplexEpoll
};


class EventLoop final {
public:
    EventLoop() = default;
    ~EventLoop() = default;

public:
    bool init(IOMultiplexType type = IOMultiplexType::IOMultiplexEpoll);

    void run();

    void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
    void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
    void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
    void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
    void unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher);

private:
    bool                            m_running{ false };
    int                             m_epollfd;

    std::unique_ptr<IIOMultiplex>   m_spIOMultiplex;
};
