#pragma once

#include <intrin.h>
#include <winternl.h>
#include "disable_windows_min_max_definetion.h"

#include <vector>
#include <stdexcept>
#include <string>
#include <Psapi.h>
#include <algorithm>
#include <regex>
#include <unordered_map>
#include <map>

#include <ppl.h>

#include "../base/types.h"
#include "../driver_control/PknDriver.h"
#include "IProcess.h"
#include "IAddressableProcess.h"
#include "../base/noncopyable.h"

namespace pkn
{
    class Driver;

    class UserProcessBase : public noncopyable
    {
    public:
        UserProcessBase(pid_t pid);
        virtual ~UserProcessBase();
    public:
        inline pid_t pid() const { return _pid; };
        bool open(DWORD access_mask = PROCESS_ALL_ACCESS);
        void close();
    public:
        inline HANDLE handle() const noexcept { return _handle; };
    private:
        pid_t _pid;
        HANDLE _handle = nullptr;
    };

    class UserBasicProcess : virtual public UserProcessBase, virtual public IProcessBasic
    {
    public:
        UserBasicProcess(pid_t pid);
        virtual ~UserBasicProcess() override = default;
        bool init();
    public:
        virtual erptr_t base() const override;
        virtual bool alive() const override;
        virtual pid_t pid() const override;
    protected:
        erptr_t _base;
    };

    class UserReadableProcess : virtual public UserProcessBase, virtual public IProcessReader
    {
    public:
        UserReadableProcess(pid_t pid) : UserProcessBase(pid) {}
        virtual ~UserReadableProcess() override = default;
    public:
        virtual bool read_unsafe(const erptr_t &address, size_t size, void *buffer) const override;
    };

    class UserWritableProcess : virtual public UserProcessBase, virtual public IProcessWriter
    {
    public:
        UserWritableProcess(pid_t pid) : UserProcessBase(pid) {}
        virtual ~UserWritableProcess() override = default;
    public:
        virtual bool write_unsafe(erptr_t address, size_t size, const void *buffer) const override;
    };

    class UserProcessRegions : virtual public UserProcessBase, virtual public IProcessRegions
    {
    public:
        UserProcessRegions(pid_t pid) : UserProcessBase(pid) {}
        virtual ~UserProcessRegions() override = default;
        // unable to known if success
        void init()
        {
            return IProcessRegions::init();
        }
    public:
        virtual MemoryRegions get_all_memory_regions() override;
        virtual bool get_mapped_file(erptr_t remote_address, estr_t *mapped_file) const override;
    };

    class UserExtraProcess : virtual public UserProcessBase, virtual public IProcessExtra
    {
    public:
        UserExtraProcess(pid_t pid) : UserProcessBase(pid) {}
        virtual ~UserExtraProcess() override = default;
    public:
        virtual erptr_t get_peb_address() const override;
        virtual erptr_t get_teb_address(pid_t thread_id) const override;
    };

    class UserProcess
        : public virtual UserBasicProcess,
        public virtual UserReadableProcess,
        public virtual UserWritableProcess,
        public virtual UserProcessRegions,
        public virtual ProcessAddressTypeInfo
    {
    public:
        UserProcess(pid_t pid) :
            UserProcessBase(pid),
            UserBasicProcess(pid),
            UserReadableProcess(pid),
            UserWritableProcess(pid),
            UserProcessRegions(pid),
            ProcessAddressTypeInfo()
        {
        }
        bool init()
        {
            if (!UserProcessBase::open())
                return false;
            if (!UserBasicProcess::init())
                return false;
            UserProcessRegions::init();
            ProcessAddressTypeInfo::init(this, this);
            return true;
        }
        virtual ~UserProcess() override = default;
    };
}
