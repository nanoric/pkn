#pragma once


class TimerGuard
{
public:
    uint64_t last_enter = 0;
    int interval_ms;
public:
    TimerGuard(int interval_ms)
        :interval_ms(interval_ms)
    {}

    bool try_enter()
    {
        static uint64_t frequency = get_frequency();
        uint64_t current;
        QueryPerformanceCounter((PLARGE_INTEGER)&current);
        if ((current - last_enter) * 1000 / frequency > interval_ms)
        {
            last_enter = current;
            return true;
        }
        return false;
    }

    static uint64_t get_frequency()
    {
        uint64_t frequency;
        QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
        return frequency;
    }

    void reset()
    {
        uint64_t current;
        QueryPerformanceCounter((PLARGE_INTEGER)&current);
        last_enter = current;
    }
};
