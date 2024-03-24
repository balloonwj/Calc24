#pragma once

#include "IOMultiplex.h"

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <vector>

/*
 int poll(struct pollfd *fds, nfds_t nfds, int timeout);
*/

class Poll : public IIOMultiplex {
public:
    virtual void poll(int timeoutUs, std::vector<IEventDispatcher*>& triggeredEventDispatchers) override {}

    virtual void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override {}
    virtual void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override {}

    virtual void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent) override {}
    virtual void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent) override {}
    virtual void unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher) override {}

private:
    //struct pollfd* m_pollfd;
    //key: fd
    //std::map<int, IEventDispatcher*>    m_eventDispatchers;
};