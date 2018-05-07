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
        static ProcessUtils* instance()
        {
            static std::mutex m;

            if (_instance != nullptr)
                return _instance;

            std::lock_guard<std::mutex> l(m);
            if (_instance != nullptr)
                return _instance;

            _instance = new KernelProcessUtils;
            return _instance;
        }
    public:
        virtual ~KernelProcessUtils() override {}
    protected:
        virtual estr_t get_process_name(euint64_t pid) override
        {
            return SingletonInjector<Driver>::get().get_process_name(pid);
        }
    };
}
