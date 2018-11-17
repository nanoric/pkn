#pragma once

#include "../base/types.h"

#include "../base/abstract/abstract.h"

namespace pkn
{
    class IBasicProcess
    {
    public:
        virtual ~IBasicProcess() = default;
    public:
        virtual pid_t pid() const PURE_VIRTUAL_FUNCTION_BODY;
        virtual erptr_t base() const PURE_VIRTUAL_FUNCTION_BODY;
        virtual bool alive() const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IReadableProcess
    {
    public:
        virtual ~IReadableProcess() = default;
    public:
        virtual bool read_unsafe(erptr_t address, size_t size, void *buffer) const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IWritableProcess
    {
    public:
        virtual ~IWritableProcess() = default;
    public:
        virtual bool write_unsafe(erptr_t address, size_t size, const void *buffer) const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IExtraProcess
    {
    public:
        virtual ~IExtraProcess() = default;
    public:
        // nullptr means failed
        virtual erptr_t get_peb_address() const PURE_VIRTUAL_FUNCTION_BODY;
        // nullptr means failed
        virtual erptr_t get_teb_address(pid_t tid) const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IProcessMemory
    {
    public:
        virtual ~IProcessMemory() = default;
    public:
        virtual bool allocate(size_t size, erptr_t *address, size_t *allocated_size = nullptr) const PURE_VIRTUAL_FUNCTION_BODY;
        virtual bool protect(erptr_t address, size_t size, uint32_t protect, uint32_t *old_protect = nullptr) const PURE_VIRTUAL_FUNCTION_BODY;
        virtual bool free(erptr_t address) const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IProcessThread
    {
    public:
        virtual ~IProcessThread() = default;
    public:
        virtual bool create_thread(erptr_t start_address, erptr_t param, pid_t *thread_id) const PURE_VIRTUAL_FUNCTION_BODY;
        virtual bool wait_for_thread(pid_t tid, uint64_t timeout_nanosec, NTSTATUS *result) const PURE_VIRTUAL_FUNCTION_BODY;
    };
}
