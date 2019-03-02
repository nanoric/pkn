#pragma once

#include <pkn/core/base/types.h>
#include <pkn/core/driver_control/PknDriver.h>
#include <pkn/core/injector/injector.hpp>
#include <pkn/core/remote_process/IProcess.h>
#include <pkn/core/remote_process/KernelProcess.h>
#include <pkn/core/remote_process/KernelProcessUtils.h>
#include <pkn/core/reader/TypedReader.hpp>
#include <pkn/core/writer/TypedWriter.hpp>

#define INIT(func_name) if(!func_name()) {printf("[-] failed to intialize function: %s\n", #func_name); return false;}

namespace pkn
{
/*
usage:
@code
estr_t process_name = make_estr("Process");
process_name += make_estr("Name.exe");
Environment env(process_name);
if(env.init())
{
    ....;
}
@endcode

Environment instance can be destroied after init() is called.
Because all its member will be record using pkn::SingletonInjector
*/

class Environment
{
public:
    Environment(const estr_t &process_name)
        :process_name(process_name)
    {}
public:
    inline bool init()
    {
        INIT(init_driver);
        INIT(init_process_utils);
        INIT(init_process_interface);
        return true;
    }
protected:
    inline bool init_driver()
    {
        using namespace pkn;

        driver = new PknDriver;
        if (!driver->open())
            return false;

        SingletonInjector<PknDriver>::set(driver);
        return true;
    }

    inline bool init_process_utils()
    {
        using namespace pkn;
        process_utils = new KernelProcessUtils();
        pkn::SingletonInjector<ProcessUtils>::set(process_utils);
        pkn::SingletonInjector<KernelProcessUtils>::set(process_utils);
        return true;
    }

    inline bool init_process_interface()
    {
        using namespace pkn;
        if (auto pid = process_utils->pid_from_process_name(process_name))
        {
            process = new KernelProcess(*pid);
            if (process->init())
            {
                pkn::SingletonInjector<KernelProcess>::set(process);
                pkn::SingletonInjector<IProcessBasic>::set(process);
                pkn::SingletonInjector<IProcessReader>::set(process);
                pkn::SingletonInjector<IProcessWriter>::set(process);
                pkn::SingletonInjector<IProcessRegions>::set(process);
                pkn::SingletonInjector<IProcessExtra>::set(process);
                pkn::SingletonInjector<IProcessMemory>::set(process);
                pkn::SingletonInjector<IProcessThread>::set(process);
                pkn::SingletonInjector<ProcessAddressTypeInfo>::set(process);

                auto tr = new TypedReader(process);
                pkn::SingletonInjector<TypedReader>::set(tr);
                auto tw = new TypedWriter(process);
                pkn::SingletonInjector<TypedWriter>::set(tw);
                return true;
            }
        }
        return false;
    }
public:
    estr_t process_name;
public:
    pkn::KernelProcessUtils *process_utils = nullptr;
    pkn::PknDriver *driver = nullptr;
    pkn::KernelProcess *process = nullptr;
};

}
