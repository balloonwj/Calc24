#pragma once

#include <functional>
#include <string>

#include "EventLoop.h"
#include "IEventDispatcher.h"

class Acceptor;
using AcceptCallback = std::function<void(int clientfd)>;

class Acceptor final : public IEventDispatcher {
public:
    Acceptor(EventLoop* pEventLoop);
    ~Acceptor();

    //...

    void setAcceptCallback(AcceptCallback&& callback) {
        m_acceptCallback = callback;
    }

public:
    virtual void onRead() override;

    bool startListen(const std::string& ip = "", uint16_t port = 8888);

private:
    virtual void onWrite() override {}

    virtual void onClose() override {}

    virtual void enableReadWrite(bool read, bool write) override {}




private:
    int                 m_listenfd;

    EventLoop* m_pEventLoop;

    AcceptCallback      m_acceptCallback;
};
