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
        virtual bool get_process_name(pid_t pid, estr_t *process_name) const noexcept override
        {
            return SingletonInjector<Driver>::get().get_process_name(pid, process_name);
        }
    };
}
