#pragma once

#include "../ps.h"

#include <ntdef.h>
#include <ntifs.h>

#include "GuardedFixer.hpp"

class Process
{
public:
    Process(uint64_t pid) :pid(pid)
    {}
    ~Process()
    {
        close();
    }
    inline bool open()
    {
        if (_hprocess != nullptr)
            return true;
        _status = PsLookupProcessByProcessId((HANDLE)pid, &_peprocess);
        if (NT_SUCCESS(_status))
        {
            _status = ObOpenObjectByPointer(_peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &_hprocess);
        }
        return NT_SUCCESS(_status);
    }
    inline void close()
    {
        if (_hprocess)
        {
            ZwClose(_hprocess);
            _hprocess = nullptr;
        }
        if (_peprocess)
        {
            ObDereferenceObject(_peprocess);
            _peprocess = nullptr;
        }
    }
    inline PEPROCESS eprocess() const { return _peprocess; }
    inline HANDLE handle() const { return _hprocess; }
    inline NTSTATUS status() const { return _status; }
public:
    NTSTATUS _status;
private:
    PEPROCESS _peprocess = nullptr;
    HANDLE _hprocess = nullptr;
    uint64_t pid;
};

class AttachProcessFixer
{
public:
    AttachProcessFixer(const Process *p)
        :p(p)
    {
    }
public:
    inline bool do_fix()
    {
        KeStackAttachProcess(p->eprocess(), &apc_state);
        return true;
    }
    inline bool do_unfix()
    {
        KeUnstackDetachProcess(&apc_state);
        return true;
    }
    inline NTSTATUS status() const { return _status; }
public:
    NTSTATUS _status = STATUS_SUCCESS;
private:
    const Process *p;
    KAPC_STATE apc_state;
};

using AttachProcess = GuardedFixer<AttachProcessFixer>;
