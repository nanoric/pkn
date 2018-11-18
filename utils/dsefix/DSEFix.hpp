#pragma once
#include <optional>

#include "../../core/base/types.h"
#include "../../core/injector/injector.hpp"
#include "../../core/remote_process/ProcessUtils.h"
#include "../PdbHelper/PdbHelper.h"

namespace pkn
{
class DSEFixAdaptor
{
public:
    ~DSEFixAdaptor() = default;
public:
    virtual std::optional<uint32_t> read_system_32bit(erptr_t address) = 0;
    virtual bool write_system_32bit(erptr_t address, uint32_t value) = 0;
};
class DSEFix
{
public:
    DSEFix(DSEFixAdaptor *adaptor)
        : _adaptor(adaptor)
    {}
    ~DSEFix()
    {
        enable();
    }
    bool init()
    {
        // global process utils
        auto &pu = SingletonInjector<ProcessUtils>::get();

        // try to get base of ntoskrnl
        if (auto m = pu.kernel_modulei(make_estr("ntoskrnl.exe")); m)
        {
            ntoskrnl = m->base;
        }
        else
        {
            return false;
        }
        ;

        // pdb for ntoskrnl
        PdbHelper pdb_ntoskrnl;
        if (!pdb_ntoskrnl.init(make_estr("C:\\Windows\\system32\\ntoskrnl.exe"), make_estr("pdb")))
            return false;

        // try to get rva for g_CiEnabled / g_CiOptions
        if (auto res = pdb_ntoskrnl.symbol_address(make_estr("g_CiEnabled")); res)
        {
            // if there is g_CiEnabled, use it
            CiEnabled = (rptr_t)*res;
        }
        else
        {
            // otherwise, use g_CiOptions
            // try to get base of ci
            if (auto m = pu.kernel_modulei(make_estr("CI.DLL")); m)
                ci = m->base;
            else
                return false;

            // pdb for ci
            PdbHelper pdb_ci;
            if (!pdb_ci.init(make_estr("C:\\Windows\\system32\\ci.dll"), make_estr("pdb")))
                return false;

            // try to get g_CiOptions
            if (auto res = pdb_ci.symbol_address(make_estr("g_CiOptions")); res)
            {
                CiOptions = (rptr_t)*res;
            }
        }

        if (CiOptions)
        {
            target_address = ci + CiOptions;
            target_value = 8;
        }
        else
        {
            target_address = ntoskrnl + CiEnabled;
            target_value = 0;
        }


        return true;
    }
public:

    // try to read current option value
    std::optional<uint32_t> read()
    {
        return _adaptor->read_system_32bit(target_address);
    }

    // disable DSE
    bool disable()
    {
        if (!disabled)
        {
            return disabled = disable_no_check();
        }
        return disabled;
    }
    bool enable()
    {
        if (disabled)
        {
            disabled = !enable_no_check();
            return !disabled;
        }
        return true;
    }
protected:
    bool disable_no_check()
    {
        if (auto res = _adaptor->read_system_32bit(target_address); !res)
        {
            org_value = *res;
            return false;
        }
        if (!_adaptor->write_system_32bit(target_address, target_value))
            return false;
        return true;
    }
    bool enable_no_check()
    {
        if (auto res = _adaptor->write_system_32bit(target_address, org_value); !res)
        {
            return false;
        }
        return true;
    }
private:
    bool disabled = false;
public:
    DSEFixAdaptor *_adaptor;
    erptr_t ntoskrnl = 0;
    erptr_t ci = 0;
    erptr_t CiOptions = 0;
    erptr_t CiEnabled = 0;
    erptr_t target_address;
    uint32_t target_value, org_value;
};
}
