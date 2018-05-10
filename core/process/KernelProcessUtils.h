#pragma once

#include <vector>

#include "../base/types.h"
#include "../driver_control/Driver.h"

#include "../injector/injector.hpp"

#include "ProcessUtils.h"
namespace pkn
{
    class KernelProcessUtils : public ProcessUtils
    {
    public:
        virtual ~KernelProcessUtils() override {}
    protected:
        virtual estr_t get_process_name(euint64_t pid) override
        {
            return SingletonInjector<Driver>::get().get_process_name(pid);
        }
    };
}
