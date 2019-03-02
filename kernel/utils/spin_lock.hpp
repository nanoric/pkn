#pragma once
#include <atomic>

class spin_lock
{
private:
    friend class spin_lock_guard;
    std::atomic_flag _lock = ATOMIC_FLAG_INIT;
};

class spin_lock_guard
{
public:
    spin_lock_guard(spin_lock &spin_lock) : _spin_lock(spin_lock)
    {
        lock();
    }
    ~spin_lock_guard()
    {
        try_release();
    }
public:
    inline void lock()
    {
        if (!acquired)
        {
            do_lock();
            acquired = true;
        }
    }
    inline void try_release()
    {
        if (acquired)
        {
            acquired = false;
            do_release();
        }
    }
private:
    inline void do_lock()
    {
        while (!_spin_lock._lock.test_and_set(std::memory_order::memory_order_acquire))
        {
        // spin
        }
        //KeAcquireSpinLock(&_spin_lock._lock, &old_irq);
    }
    inline void do_release()
    {
        _spin_lock._lock.clear(std::memory_order::memory_order_release);
        //KeReleaseSpinLock(&_spin_lock._lock, old_irq);
    }
    spin_lock &_spin_lock;
    KIRQL old_irq;
    bool acquired = false;
};
