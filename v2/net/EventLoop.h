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
#include "ITimerService.h"
#include "Timer.h"
#include "WakeupEventDispatcher.h"

 //using CustomTask = std::function<bool(const std::string&)>;

using CustomTask = std::function<void()>;

enum class IOMultiplexType {
    IOMultiplexTypeSelect,
    IOMultiplexTypePoll,
    IOMultiplexEpoll
};

class EventLoop final : public ITimerService {
public:
    EventLoop(bool isBaseLoop = false) : m_isBaseLoop(isBaseLoop) {}
    ~EventLoop();

public:
    bool init(IOMultiplexType type = IOMultiplexType::IOMultiplexEpoll);

    void run();

    void addTask(const CustomTask& task);

    void setThreadID(const std::thread::id& threadID);
    const std::thread::id& getThreadID() const;

    bool isCallableInOwnerThread() const;

    void registerReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
    void registerWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
    void unregisterReadEvent(int fd, IEventDispatcher* eventDispatcher, bool readEvent);
    void unregisterWriteEvent(int fd, IEventDispatcher* eventDispatcher, bool writeEvent);
    void unregisterAllEvents(int fd, IEventDispatcher* eventDispatcher);

    //定时器相关接口
    virtual int64_t addTimer(int32_t intervalMs, bool repeated, int64_t repeatedCount,
        TimerTask timerTask, TimerMode mode = TimerMode::TimerModeFixedInterval) override;
    virtual void removeTimer(int64_t timerID) override;

private:
    bool createWakeupfd();

    void checkAndDoTimers();

    void doOtherTasks();

    void addTimerInternal(std::shared_ptr<Timer> spTimer);
    void removeTimerInternal(int64_t timerID);

private:
    bool                                    m_running{ false };
    int                                     m_epollfd;
    int                                     m_wakeupfd;

    bool                                    m_isBaseLoop{ false };

    std::unique_ptr<IIOMultiplex>           m_spIOMultiplex;
    std::thread::id                         m_threadID;

    WakeupEventDispatcher* m_pWakeupEventDispatcher{ nullptr };

    std::vector<CustomTask>                 m_customTasks;
    std::mutex                              m_mutexTasks;

    std::vector<std::shared_ptr<Timer>>     m_timers;
    std::mutex                              m_mutexTimers;

    bool                                    m_isCheckTimers{ false };
    std::vector<std::shared_ptr<Timer>>     m_pendingAddTimers;
    std::vector<int64_t>                    m_pendingRemoveTimers;
};
