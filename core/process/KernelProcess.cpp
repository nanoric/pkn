#include "../driver_control/Driver.h"
#include "../injector/injector.hpp"

#include "KernelProcess.h"

namespace pkn
{
    KernelProcessBase::KernelProcessBase(pid_t pid)
        : _pid(pid), _driver(SingletonInjector<Driver>::get())

    {

    }

    KernelBasicProcess::KernelBasicProcess(pid_t pid)
        : KernelProcessBase(pid)
    {
    }

    bool KernelBasicProcess::init()
    {
        return driver().get_process_base(pid(), &_base);
    }

    erptr_t KernelBasicProcess::base() const
    {
        return this->_base;
    }

    bool KernelBasicProcess::alive() const
    {
        return driver().is_process_alive(pid());
    }

    bool KernelReadableProcess::read_unsafe(erptr_t address, size_t size, void *buffer) const
    {
        return driver().read_process_memory(pid(), address, size, buffer);
    }

    bool KernelWritableProcess::write_unsafe(erptr_t address, size_t size, const void *buffer) const
    {
        return driver().write_process_memory(pid(), address, size, buffer);
    }

    MemoryRegions KernelProcessRegions::get_all_memory_regions()
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        auto address_low = (rptr_t)si.lpMinimumApplicationAddress;
        auto address_high = (rptr_t)si.lpMaximumApplicationAddress;
        auto page_size = si.dwAllocationGranularity;

        MemoryRegions regions;
        for (rptr_t p = address_low; p < address_high;)
        {
            MEMORY_BASIC_INFORMATION mbi;
            if (!driver().virtual_query(pid(), p, &mbi))
            {
                p += page_size;
                continue;
            }

            size_t memSize = mbi.RegionSize;
            if (mbi.State == MEM_COMMIT)
            {
                MemoryRegion region;
                region.base = (rptr_t)mbi.BaseAddress;
                region.size = (size_t)mbi.RegionSize;
                region.protect = (uint32_t)mbi.Protect;
                region.allocation_base = (rptr_t)mbi.AllocationBase;
                region.type = mbi.Type;
                regions.emplace_back(region);
            }
            p = p + memSize;
        }
        return regions;
    }
    bool KernelProcessRegions::get_mapped_file(erptr_t remote_address, estr_t *mapped_file) const
    {
        return driver().get_mapped_file(pid(), remote_address, mapped_file);
    }

    erptr_t KernelExtraProcess::get_peb_address() const
    {
        return driver().get_peb_address();
    }
}

