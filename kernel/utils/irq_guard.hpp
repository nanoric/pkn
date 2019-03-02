#pragma once

class irq_guard
{
public:
    irq_guard(KIRQL target_level = APC_LEVEL)
        : _target_level(target_level)
    {
        lock();
    }
    ~irq_guard()
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
        KeRaiseIrql(_target_level, &_old_irq);
    }
    inline void do_release()
    {
        KeLowerIrql(_old_irq);
    }
    KIRQL _target_level = 0;
    KIRQL _old_irq;
    bool acquired = false;
};
