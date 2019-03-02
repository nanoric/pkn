#pragma once

template <class Fixer>
class GuardedFixer
{
public:
    template <class ... Args>
    GuardedFixer(Args ... args)
        : fixer(args ... )
    {}
    ~GuardedFixer()
    {
        if (is_fixed())
            unfix();
    }

public:
    inline bool is_fixed() { return _is_fixed; };
    inline bool fix()
    {
        if (is_fixed())
            return true;
        _is_fixed = fixer.do_fix();
        return _is_fixed;
    }
    inline bool unfix()
    {
        if (!is_fixed())
            return true;
        _is_fixed = !fixer.do_unfix();
        return !_is_fixed;
    }
private:
    Fixer fixer;
    bool _is_fixed = false;
};
