#pragma once

#include "IOMultiplex.h"

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <vector>

/*
 int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
*/

class Epoll : public IIOMultiplex {
public:
    Epoll();
    virtual ~Epoll();

public:
    virtual void poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers) override;

    virtual void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override;
    virtual void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override;

    virtual void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override;
    virtual void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override;
    virtual void unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher) override;

private:
    Epoll(const Epoll& rhs) = delete;
    Epoll& operator=(const Epoll& rhs) = delete;

    Epoll(Epoll&& rhs) = delete;
    Epoll& operator=(Epoll&& rhs) = delete;

private:
    int                         m_epollfd{ -1 };
    //key=>fd, value=eventFlags
    std::map<int, int32_t>      m_fdEventFlags;
};