#pragma once

#include <stdint.h>
#include "Process.h"

NTKERNELAPI HANDLE MmSecureVirtualMemory(
    PVOID  Address,
    SIZE_T Size,
    ULONG  ProbeMode
);

NTKERNELAPI VOID MmUnsecureVirtualMemory(
    HANDLE SecureHandle
);

/*
Prevent memory from being free and make its page protection cannot be made more restrictive.
But is not absolutely: If process terminate, that memory will still be freed.
@note use with AttachProcess(), or use ProcessSecureMemoryFixer()
usage:
Process p(pid);
if(p.open()
{
    AttachProcess attach(p)
    if(attach.fix())
    {
        SecureMemory sec(address, size, PAGE_READWRITE);
        if(sec.fix())
        {
         ....
         }
    }
}
*/
class SecureMemoryFixer
{
public:
    SecureMemoryFixer(void *address, size_t size, uint32_t probe_mode)
        :address(address), size(size), probe_mode(probe_mode)
    {}
    bool do_fix()
    {
        MmSecureVirtualMemory(address, size, probe_mode);
        return secure_handle;
    }
    bool do_unfix()
    {
        MmUnsecureVirtualMemory(secure_handle);
        return true;
    }

public:
    void *address;
    size_t size;
    uint32_t probe_mode;
    HANDLE secure_handle;
};

class ProcessSecureMemoryFixer
{
public:
    ProcessSecureMemoryFixer(uint64_t pid, void *address, size_t size, uint32_t probe_mode)
        :p(pid), attach(&p), address(address), size(size), probe_mode(probe_mode)
    {}
    bool do_fix()
    {
        if (p.open())
        {
            if (attach.fix())
            {
                secure_handle = MmSecureVirtualMemory(address, size, probe_mode);
                return secure_handle;
            }
        }
        return false;
    }
    bool do_unfix()
    {
        if(secure_handle)
            MmUnsecureVirtualMemory(secure_handle);
        attach.unfix();
        p.close();
        return true;
    }
public:
    Process p;
    AttachProcess attach;
    void *address;
    size_t size;
    uint32_t probe_mode;
    HANDLE secure_handle = nullptr;
};

using ProcessSecureMemory = GuardedFixer<ProcessSecureMemoryFixer>;
using SecureMemory = GuardedFixer<SecureMemoryFixer>;
