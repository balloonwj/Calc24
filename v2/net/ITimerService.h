#pragma once

#include <cstdint>
#include <functional>

using TimerTask = std::function<void(int64_t timerID)>;

enum class TimerMode {
    TimerModeFixedInterval = 0,
    TimerModeFixedDelay = 1
};

class ITimerService {
public:
    virtual int64_t addTimer(int32_t intervalMs, bool repeated, int64_t repeatedCount,
        TimerTask timerTask, TimerMode mode) = 0;
    virtual void removeTimer(int64_t timerID) = 0;
};
