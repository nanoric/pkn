#pragma once

#include <intrin.h>

#include <pkn/core/base/types.h>

#include "GuardedFixer.hpp"

// use with irq_guard irq(DISPATCH_LEVEL)
class DisablePageProtectFixer
{
public:
    bool init()
    {
        return true;
    }
    bool do_fix()
    {
        org_cr0 = __readcr0();
        __writecr0(org_cr0 & 0xFFFFFFFFFFFEFFFFull);
        return true;
    }
    bool do_unfix()
    {
        __writecr0(org_cr0);
        return false;
    }
private:
    uint64_t org_cr0;
};

using DisablePageProtect = GuardedFixer<DisablePageProtectFixer>;
