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
    class PknDriver;

    class KernelProcessBase : public noncopyable
    {
    public:
        KernelProcessBase(pid_t pid);
        virtual ~KernelProcessBase() = default;
    public:
        inline pid_t pid() const { return _pid; };
        inline PknDriver &driver() const { return _driver; }
    private:
        pid_t _pid = 0;
        PknDriver &_driver;
    };

    class KernelBasicProcess : virtual public KernelProcessBase, virtual public IProcessBasic
    {
    public:
        KernelBasicProcess(pid_t pid);
        virtual ~KernelBasicProcess() override = default;
    public:
        bool init();
        virtual pid_t pid() const override { return KernelProcessBase::pid(); }
        virtual erptr_t base() const override;
        virtual bool alive() const override;
    protected:
        erptr_t _base;
    };

    class KernelReadableProcess : virtual public KernelProcessBase, virtual public IProcessReader
    {
    public:
        KernelReadableProcess(pid_t pid) : KernelProcessBase(pid) {}
        virtual ~KernelReadableProcess() override = default;
    public:
        virtual bool read_unsafe(const erptr_t &address, size_t size, void *buffer) const noexcept override;
    };

    class KernelWritableProcess : virtual public KernelProcessBase, virtual public IProcessWriter
    {
    public:
        KernelWritableProcess(pid_t pid) : KernelProcessBase(pid) {}
        virtual ~KernelWritableProcess() override = default;
    public:
        virtual bool write_unsafe(erptr_t address, size_t size, const void *buffer) const noexcept override;
        virtual bool force_write(erptr_t address, size_t size, const void *buffer) const noexcept override;
        virtual bool acquire_lock(const erptr_t &lock_address) const noexcept override;
    };

    class KernelProcessRegions : virtual public KernelProcessBase, virtual public IProcessRegions
    {
    public:
        KernelProcessRegions(pid_t pid) : KernelProcessBase(pid) {}
        virtual ~KernelProcessRegions() override = default;
    public:
        void init()
        {
            IProcessRegions::init();
        }
        virtual MemoryRegions get_all_memory_regions() override;
        virtual bool get_mapped_file(erptr_t remote_address, estr_t *mapped_file) const override;
    };

    class KernelExtraProcess : virtual public KernelProcessBase, virtual public IProcessExtra
    {
    public:
        KernelExtraProcess(pid_t pid) : KernelProcessBase(pid) { }
        virtual ~KernelExtraProcess() override = default;
    public:
        virtual erptr_t get_peb_address() const override;
        virtual erptr_t get_teb_address(pid_t tid) const override;
    };

    class KernelProcessMemory : virtual public KernelProcessBase, virtual public IProcessMemory
    {
    public:
        KernelProcessMemory(pid_t pid) : KernelProcessBase(pid) { }
        virtual ~KernelProcessMemory() override = default;
    public:
        virtual bool allocate(size_t size, erptr_t *address, size_t *allocated_size = nullptr, MemoryProtect protect = MemoryProtect::ReadWriteExecute) const override;
        virtual bool protect(erptr_t address, size_t size, uint32_t protect, uint32_t *old_protect = nullptr) const override;
        virtual bool free(erptr_t address) const override;
    };

    class KernelProcessThread : virtual public KernelProcessBase, virtual public IProcessThread
    {
    public:
        KernelProcessThread(pid_t pid) : KernelProcessBase(pid) {}
        virtual ~KernelProcessThread() override = default;
    public:
        virtual bool create_thread(erptr_t start_address, erptr_t param, pid_t *thread_id) const override;
        virtual bool wait_for_thread(pid_t tid, uint64_t timeout_nanosec, NTSTATUS *result) const override;

    };

    class KernelProcess
        : public virtual KernelBasicProcess,
        public virtual KernelReadableProcess,
        public virtual KernelWritableProcess,
        public virtual KernelProcessRegions,
        public virtual KernelExtraProcess,
        public virtual KernelProcessMemory,
        public virtual KernelProcessThread,
        public virtual ProcessAddressTypeInfo
    {
    public:
        KernelProcess(pid_t pid) :
            KernelProcessBase(pid),
            KernelBasicProcess(pid),
            KernelReadableProcess(pid),
            KernelWritableProcess(pid),
            KernelProcessRegions(pid),
            KernelExtraProcess(pid),
            KernelProcessMemory(pid),
            KernelProcessThread(pid)
        {
        }
        bool init()
        {
            if (!KernelBasicProcess::init())
                return false;
            KernelProcessRegions::init();
            ProcessAddressTypeInfo::init(this, this);
            return true;
        }
        virtual ~KernelProcess() override = default;
    };
}
