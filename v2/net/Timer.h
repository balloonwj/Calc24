#pragma once

#include <cstdint>

#include "ITimerService.h"

class Timer
{
public:
    Timer(int32_t intervalMs, bool repeated, int64_t repeatedCount,
        TimerTask timerTask, TimerMode mode = TimerMode::TimerModeFixedInterval);
    ~Timer() = default;

    int64_t id() const {
        return m_id;
    }

    int64_t nextTriggeredTimeMs() {
        return m_nextTriggeredTimeMs;
    }

    void doTimer(int64_t timerID, int64_t nowMs);

private:
    Timer(const Timer& rhs) = delete;
    Timer& operator=(const Timer& rhs) = delete;

    static int64_t generateTimerID();

    //TODO: 
    //move constructor
    //move operator=

private:
    int64_t     m_id;
    int32_t     m_intervalMs; //定时器时间间隔
    bool        m_repeated;
    TimerMode   m_mode;
    int64_t     m_repeatedCount;
    TimerTask   m_timerTask;

    int64_t     m_nextTriggeredTimeMs;
};
