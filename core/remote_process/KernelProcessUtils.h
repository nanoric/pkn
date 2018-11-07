#pragma once

#include <vector>

#include "../base/types.h"
#include "../driver_control/PknDriver.h"

#include "../injector/injector.hpp"

#include "ProcessUtils.h"
namespace pkn
{
    /*
    If you want to use this class, you should:
    ** Setup SingletonInjector<Driver> first! **

    */
    class KernelProcessUtils : public ProcessUtils
    {
    public:
        virtual ~KernelProcessUtils() override {}
    protected:
        virtual bool query_system_information(uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept override
        {
            return SingletonInjector<PknDriver>::get().query_system_information(informaiton_class, buffer, buffer_size, ret_size);
        }

        virtual std::optional<estr_t> get_process_name(euint64_t pid) const noexcept override
        {
            return SingletonInjector<PknDriver>::get().get_process_name(pid);
        }
    };
}
