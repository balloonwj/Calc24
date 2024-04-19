#include "Timer.h"

#include <atomic>
#include <chrono>

Timer::Timer(int32_t intervalMs, bool repeated,
    int64_t repeatedCount, TimerTask timerTask,
    TimerMode mode/* = TimerMode::TimerModeFixedInterval*/) : m_intervalMs(intervalMs),
    m_repeated(repeated),
    m_mode(mode),
    m_repeatedCount(repeatedCount),
    m_timerTask(timerTask) {

    int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    m_nextTriggeredTimeMs = nowMs + m_intervalMs;

    m_id = Timer::generateTimerID();
}

void Timer::doTimer(int64_t timerID, int64_t nowMs) {
    if (m_repeated) {
        if (m_mode == TimerMode::TimerModeFixedInterval) {
            //m_intervalMs=1s => 2s
            //m_nextTriggeredTimeMs = 1 
            //nowMs = 1 => 3s
            //
            m_nextTriggeredTimeMs += m_intervalMs;
        } else {
            m_nextTriggeredTimeMs = nowMs + m_intervalMs;
        }

    }

    //对于TimerModeFixedInterval模式：
    //假设定时器的时间间隔是1s，某次执行定时器回调花了2s，下一次触发的时候当前时间是第3秒，需要连续执行2次定时器回调函数
    // 1s、(2s）、3s
    //对于TimerModeFixedDelay模式：
    //假设定时器的时间间隔是1s，某次执行定时器回调花了2s，下一次触发的时候当前时间是第3秒，需要连续执行1次定时器回调函数
    //1s、2s（3s）、5s
    m_timerTask(timerID);
}

int64_t Timer::generateTimerID() {
    static std::atomic<int64_t> baseID{ 0 };
    baseID++;

    return baseID;
}

