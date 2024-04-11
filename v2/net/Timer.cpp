#include "Timer.h"

#include <atomic>
#include <chrono>

Timer::Timer(int32_t intervalMs, bool repeated,
    int64_t repeatedCount, TimerTask timerTask) : m_intervalMs(intervalMs),
    m_repeated(repeated),
    m_repeatedCount(repeatedCount),
    m_timerTask(timerTask) {

    int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    m_nextTriggeredTimeMs = nowMs + m_intervalMs;

    m_id = Timer::generateTimerID();
}

void Timer::doTimer(int64_t timerID) {
    if (m_repeated) {
        m_nextTriggeredTimeMs += m_intervalMs;
    }

    m_timerTask(timerID);
}

int64_t Timer::generateTimerID() {
    static std::atomic<int64_t> baseID{ 0 };
    baseID++;

    return baseID;
}

