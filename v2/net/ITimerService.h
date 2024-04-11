#pragma once

#include <cstdint>
#include <functional>

using TimerTask = std::function<void(int64_t timerID)>;

class ITimerService {
public:
    virtual int64_t addTimer(int32_t intervalMs, bool repeated, int64_t repeatedCount, TimerTask timerTask) = 0;
    virtual bool removeTimer(int64_t timerID) = 0;
};
