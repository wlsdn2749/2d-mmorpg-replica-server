// TimeUtil.h
#pragma once
#include <chrono>
#include <thread>
#include "Types.h"

namespace Time
{
    using Steady = std::chrono::steady_clock;
    using System = std::chrono::system_clock;

    // === Monotonic time: 틱/쿨다운/타이머용 (시스템 시간 변경 영향 X) ===
    inline int64 NowSteadyMs()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(Steady::now().time_since_epoch()).count();
    }
    inline int64 NowSteadyUs()
    {
        using namespace std::chrono;
        return duration_cast<microseconds>(Steady::now().time_since_epoch()).count();
    }
    inline int64 NowSteadyNs()
    {
        using namespace std::chrono;
        return duration_cast<nanoseconds>(Steady::now().time_since_epoch()).count();
    }

    // === Wall-clock: 로그/타임스탬프 표시용 (시스템 시간 변경 영향 O) ===
    inline int64 NowSystemMs()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(System::now().time_since_epoch()).count();
    }

    // === Sleep helpers ===
    inline void SleepForMs(int64 ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    // === Duration 변환 편의 ===
    template <class Rep, class Period>
    inline int64 ToMs(std::chrono::duration<Rep, Period> d)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
    }

    // 간단 스톱워치 (프로파일링/쿨다운 측정 등)
    class Stopwatch
    {
    public:
        Stopwatch() : _start(NowSteadyMs()) {}
        void  Reset() { _start = NowSteadyMs(); }
        int64 ElapsedMs() const { return NowSteadyMs() - _start; }
    private:
        int64 _start;
    };
}

// 편의 매크로 (선호 시)
#ifndef NOW_MS
#define NOW_MS() Time::NowSteadyMs()
#endif
