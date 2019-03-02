#pragma once 
#include "../base/types.h"
#include "../base/abstract/abstract.h"

typedef _Return_type_success_(return >= 0) long NTSTATUS;

#ifndef PAGE_READONLY
#define PAGE_READONLY 0x02
#endif

#ifndef PAGE_READWRITE
#define PAGE_READWRITE 0x04
#endif

#ifndef PAGE_EXECUTE_READ
#define PAGE_EXECUTE_READ 0x20
#endif

#ifndef PAGE_EXECUTE_READWRITE
#define PAGE_EXECUTE_READWRITE 0x40
#endif

namespace pkn
{
// used for specific the protection of memory to allocate
enum class MemoryProtect
{
    ReadOnly = PAGE_READONLY,
    ReadWrite = PAGE_READWRITE,
    ReadExecute = PAGE_EXECUTE_READ,
    ReadWriteExecute = PAGE_EXECUTE_READWRITE,
};
inline uint32_t memory_protect_to_win32_protect(MemoryProtect protect) noexcept { return (uint32_t)protect; }

class IProcessBasic
{
public:
    virtual ~IProcessBasic() = default;
public:
    virtual pid_t pid() const PURE_VIRTUAL_FUNCTION_BODY;
    virtual erptr_t base() const PURE_VIRTUAL_FUNCTION_BODY;
    virtual bool alive() const PURE_VIRTUAL_FUNCTION_BODY;
};

class IProcessReader
{
public:
    virtual ~IProcessReader() = default;
public:
    virtual bool read_unsafe(const erptr_t &address, size_t size, void *buffer) const PURE_VIRTUAL_FUNCTION_BODY;
};

class IProcessWriter
{
public:
    virtual ~IProcessWriter() = default;
public:
    virtual bool write_unsafe(erptr_t address, size_t size, const void *buffer) const PURE_VIRTUAL_FUNCTION_BODY;
    virtual bool force_write(erptr_t address, size_t size, const void *buffer) const { return false; };
    virtual bool acquire_lock(const erptr_t &lock_address) const { return false; };
    bool release_lock(const erptr_t &lock_address) const { int zero = 0; return write_unsafe(lock_address, 4, &zero); }
};

class IProcessExtra
{
public:
    virtual ~IProcessExtra() = default;
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
    virtual bool allocate(size_t size, erptr_t *address, size_t *allocated_size = nullptr, MemoryProtect protect = MemoryProtect::ReadWriteExecute) const PURE_VIRTUAL_FUNCTION_BODY;
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
