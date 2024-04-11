#pragma once

#include "IEventDispatcher.h"

class WakeupEventDispatcher : public IEventDispatcher {
public:
    WakeupEventDispatcher(int fd);
    virtual ~WakeupEventDispatcher();

public:
    virtual void onRead() override;
    virtual void onWrite() override {}

    virtual void onClose() override {}

    virtual void enableReadWrite(bool read, bool write) override;

    void wakeup();


private:
    int                         m_wakeupfd;
    bool                        m_enableRead{ false };
    bool                        m_enableWrite{ false };
};
