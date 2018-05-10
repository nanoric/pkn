#pragma once
#include "../timer_guard/TimerGuard.hpp"

class FpsCounter
{
public:
    uint64_t count()
    {
        frames++;
        if (guard.try_enter())
        {
            _fps = frames;
            frames = 0;
        }
        return fps();
    }
    inline uint64_t fps() const { return _fps; }
    inline operator uint64_t() const { return fps(); }
    inline uint64_t operator++() { return count(); }
    inline uint64_t operator++(int) { return count(); }
public:
    uint64_t _fps = 0;
private:
    TimerGuard guard = TimerGuard(1000);
    uint64_t frames = 0;
};