#include <ntdef.h>
#include <ntifs.h>

#include <stdint.h>
#include <vector>

#include <pkn/core/marcos/debug_print.h>

#include "mem.h"
#include "io_code.h"
#include "ps.h"
#include "utils/system_proc.h"
#include "utils/irq_guard.hpp"
#include "utils/DisablePageProtect.hpp"
#include "utils/Process.h"
#include "utils/memutils.hpp"
#include "utils/SecureMemory.hpp"

#ifndef MEM_IMAGE
#define MEM_IMAGE 0x01000000
#endif
typedef struct _MEMORY_SECTION_NAME
{
    UNICODE_STRING Name;
    WCHAR     Buffer[260];
}MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

#ifndef MemorySectionName
#define MemorySectionName (MemoryBasicInformation + 2)
#endif




//NTSTATUS my_copy_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer)
//{
//    NTSTATUS status = STATUS_SUCCESS;
//    PEPROCESS peprocess;
//    PVOID value = 0;
//
//    status = PsLookupProcessByProcessId((HANDLE)processid, &peprocess);
//    if (NT_SUCCESS(status))
//    {
//        KAPC_STATE state;
//        KeStackAttachProcess((PKPROCESS)peprocess, &state);
//
//        RtlCopyMemory(buffer, address, size);
//            
//        KeUnstackDetachProcess(&state);
//        ObDereferenceObject(peprocess);
//    }
//    return (UINT64)value;
//}

NTSTATUS mm_copy_virtual_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer)
{
    PEPROCESS peprocess;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)processid, &peprocess);
    if (NT_SUCCESS(status))
    {
        SIZE_T ncopy;
        status = MmCopyVirtualMemory(peprocess, (PVOID)address, PsGetCurrentProcess(), (PVOID)buffer, size, UserMode, &ncopy);
        ObDereferenceObject(peprocess);
    }
    return status;
}
bool direct_copy_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer)
{
    constexpr int stack_copy_size = 0x30;
    char stackmem[stack_copy_size];
    char *systemmem = stackmem;
    bool allocate = false;
    if (size <= stack_copy_size)
    {
        systemmem = stackmem;
    }
    else
    {
        systemmem = new char[size];
        allocate = true;
    }
    // copy at stack
    memcpy(systemmem, (void *)buffer, size);
    Process p(processid);
    if (p.open())
    {
        AttachProcess attach(&p);
        if (attach.fix())
        {
            memcpy((void *)address, systemmem, size);
            if (allocate)
                delete systemmem;
            return true;
        }
        if (allocate)
            delete systemmem;
        return false;
    }
}

bool secure_copy_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer)
{
    constexpr int stack_copy_size = 0x30;
    char stackmem[stack_copy_size];
    char *systemmem = stackmem;
    bool allocate = false;
    if (size <= stack_copy_size)
    {
        systemmem = stackmem;
    }
    else
    {
        systemmem = new char[size];
        allocate = true;
    }
    // copy at stack
    memcpy(systemmem, (void *)buffer, size);
    ProcessSecureMemory sec(processid, (void *)address, size, PAGE_READONLY);
    if (sec.fix())
    {
        //memcpy((void *)address, systemmem, size);
        if (allocate)
            delete systemmem;
        return true;
    }
    if (allocate)
        delete systemmem;
    return false;
}

NTSTATUS read_process_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer)
{
    //if (secure_copy_memory(processid, address, size, buffer))
    //    return STATUS_SUCCESS;
    //return STATUS_UNSUCCESSFUL;
    return mm_copy_virtual_memory(processid, address, size, buffer);
}

//NTSTATUS read_process_memories(UINT64 processid, SIZE_T count, const ReadProcessMemoriesInputData *datas)
//{
//    NTSTATUS status = STATUS_SUCCESS;
//    int full_sucess = TRUE;
//    for (int i = 0; i < count; i++)
//    {
//        const ReadProcessMemoriesInputData *pd = &datas[i];
//        status = read_process_memory(processid, pd->startaddress, pd->bytestoread, pd->buffer);
//        if (!NT_SUCCESS(status))
//            full_sucess = FALSE;
//    }
//    if(full_sucess)
//        return status;
//    return STATUS_PARTIAL_COPY;
//}

NTSTATUS write_process_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer)
{
    PEPROCESS peprocess;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)processid, &peprocess);
    if (NT_SUCCESS(status))
    {
        SIZE_T ncopy;
        status = MmCopyVirtualMemory(PsGetCurrentProcess(), (PVOID)buffer, peprocess, (PVOID)address, size, UserMode, &ncopy);
        ObDereferenceObject(peprocess);
    }
    return status;
}
NTSTATUS query_virtual_memory(UINT64 pid, UINT64 address, MEMORY_BASIC_INFORMATION *pmbi)
{
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            SIZE_T retsize;
            status = ZwQueryVirtualMemory(hprocess, (PVOID)address, MemoryBasicInformation, pmbi, sizeof(MEMORY_BASIC_INFORMATION), &retsize);

            ZwClose(hprocess);
        }

        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS get_mapped_file(UINT64 pid, UINT64 address, wchar_t *image_path, SIZE_T size_path)
{
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            SIZE_T retsize;
            if (image_path)
            {
                MEMORY_SECTION_NAME msn;
                status = ZwQueryVirtualMemory(hprocess, (PVOID)address, (MEMORY_INFORMATION_CLASS)MemorySectionName, &msn, sizeof(msn), &retsize);
                if (NT_SUCCESS(status))
                {
                    SIZE_T ncopy = size_path - sizeof(*image_path) < retsize ? size_path - sizeof(*image_path) : msn.Name.Length;
                    RtlCopyMemory(image_path, msn.Name.Buffer, ncopy);
                    image_path[ncopy / sizeof(*image_path)] = 0;
                }
            }
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS allocate_virtual_memory(
    _In_    UINT64    pid,
    _Inout_ PVOID     *BaseAddress,
    _Inout_ PSIZE_T   RegionSize,
    _In_    ULONG     AllocationType,
    _In_    ULONG     Protect
)
{
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            status = ZwAllocateVirtualMemory(hprocess, BaseAddress, 0, RegionSize, AllocationType, Protect);
            if (NT_SUCCESS(status))
                DebugPrint("ZwAllocateVirtualMemory success ");
            else
                DebugPrint("ZwAllocateVirtualMemory failed : %x", status);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS free_virtual_memory(
    _In_    UINT64    pid,
    _Inout_ PVOID   *BaseAddress,
    _Inout_ PSIZE_T RegionSize,
    _In_    ULONG   FreeType
)
{
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            status = ZwFreeVirtualMemory(hprocess, BaseAddress, RegionSize, FreeType);
            if (NT_SUCCESS(status))
                DebugPrint("ZwFreeVirtualMemory success ");
            else
                DebugPrint("ZwFreeVirtualMemory failed : %x", status);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}


NTSTATUS protect_virtual_memory(
    _In_    UINT64    pid,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG NewProtect,
    __out PULONG OldProtect
)
{
    static fZwProtectVirtualMemory pZwProtectVirtualMemory = (fZwProtectVirtualMemory)get_system_proc(L"ZwProtectVirtualMemory");
    if (pZwProtectVirtualMemory == nullptr)
    {
        return STATUS_UNSUCCESSFUL;
    }
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            status = pZwProtectVirtualMemory(hprocess, BaseAddress, RegionSize, NewProtect, OldProtect);
            if (NT_SUCCESS(status))
                DebugPrint("ZwProtectVirtualMemory success ");
            else
                DebugPrint("ZwProtectVirtualMemory failed : %x", status);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}

std::optional<void *> allocate_nonpaged_memory(SIZE_T size)
{
    //auto retv = ExAllocatePoolWithTag(
    //    NonPagedPool,
    //    size,
    //    'Nonp'
    //);
    //return retv;




    PHYSICAL_ADDRESS low, high, skip;
    low.QuadPart = 0;
    high.QuadPart = INT64_MAX;
    skip.QuadPart = 0;
    constexpr auto padding = 0;  // add padding, avoid write volitation
    PMDL pmdl = MmAllocatePagesForMdl(low, high, skip, size + padding);
    if (nullptr == pmdl)
    {
        DebugPrint("allocate_nonpaged_memory: MmAllocatePagesForMdl failed with NULL return!");
        return std::nullopt;
    }
    auto ptr = MmGetSystemAddressForMdlSafe(pmdl, NormalPagePriority);
    if (nullptr == ptr)
    {
        DebugPrint("allocate_nonpaged_memory: MmAllocatePagesForMdl failed with NULL return!");
        return std::nullopt;
    }
    return ptr;
}

size_t copy_virtual_memory(void *dst, void *src, size_t size)
{
    //MM_COPY_ADDRESS mca;
    //mca.VirtualAddress = src;
    //size_t ncopy;
    //if (!MmIsAddressValid(dest))
    //    return 0;
    //if (!MmIsAddressValid(src))
    //    return 0;
    //auto last = (char *)dest + size - 1;
    //if (!MmIsAddressValid(last))
    //    return 0;
    //MmCopyMemory(dest, mca, size, MM_COPY_MEMORY_VIRTUAL, &ncopy);
    //return ncopy;
    memcpy(dst, src, size);
    return size;
}

bool lock_memory(uint64_t pid, void *dest, size_t size)
{
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            status = ZwLockVirtualMemory(hprocess, &dest, &size, MAP_SYSTEM);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return NT_SUCCESS(status);
}

bool unlock_memory(uint64_t pid, void *dest, size_t size)
{
    NTSTATUS status;
    PEPROCESS peprocess;
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        HANDLE hprocess;
        status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsProcessType, KernelMode, &hprocess);
        if (NT_SUCCESS(status))
        {
            status = ZwUnlockVirtualMemory(hprocess, &dest, &size, MAP_SYSTEM);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return NT_SUCCESS(status);
}

bool force_write_8(uint64_t pid, void *dest, uint8_t val)
{
    if (lock_memory(pid, dest, 1))
    {
        Process p(pid);
        if (p.open())
        {
            AttachProcess attach(&p);
            if (attach.fix())
            {
                irq_guard irq(DISPATCH_LEVEL);
                {
                    DisablePageProtect dpp;
                    dpp.fix();
                    typed_copy<uint8_t>(dest, &val);
                }
            }
        }
        if (unlock_memory(pid, dest, 1))
        {
            return true;
        }
        else
        {
            DebugPrint("Failed to unlock memory after lock it!\n");
        }
    }
    else
    {
        DebugPrint("Failed to lock memory while writing 1 bytes into %llx!\n", (rptr_t)dest);
    }
    return false;
}


bool force_write_64(uint64_t pid, void *dest, uint64_t val)
{
    if (lock_memory(pid, dest, 8))
    {
        Process p(pid);
        if (p.open())
        {
            AttachProcess attach(&p);
            if (attach.fix())
            {
                irq_guard irq(DISPATCH_LEVEL);
                {
                    DisablePageProtect dpp;
                    dpp.fix();
                    typed_copy<uint64_t>(dest, &val);
                }
            }
        }
        if (unlock_memory(pid, dest, 8))
        {
            return true;
        }
        else
        {
            DebugPrint("Failed to unlock memory after lock it!\n");
        }
    }
    else
    {
        DebugPrint("Failed to lock memory while writing 1 bytes into %llx!\n", (rptr_t)dest);
    }
    return false;
}

bool force_write(uint64_t pid, void * dest, void *src, size_t size)
{
    if (lock_memory(pid, dest, size))
    {
        Process p(pid);
        if (p.open())
        {
            AttachProcess attach(&p);
            if (attach.fix())
            {
                irq_guard irq(DISPATCH_LEVEL);
                {
                    DisablePageProtect dpp;
                    dpp.fix();
                    copy_virtual_memory(dest, src, size);
                }
            }
        }
        if (unlock_memory(pid, dest, 8))
        {
            return true;
        }
        else
        {
            DebugPrint("Failed to unlock memory after lock it!\n");
        }
    }
    else
    {
        DebugPrint("Failed to lock memory while writing 1 bytes into %llx!\n", (rptr_t)dest);
    }
    return false;
}

bool is_address_valid(void *address)
{
    //MEMORY_BASIC_INFORMATION mbi;
    //size_t out_size = 0;
    //NTSTATUS status = ZwQueryVirtualMemory(p.handle(),
    //                                       reinterpret_cast<void *>(dest),
    //                                       MemoryBasicInformation,
    //                                       &mbi,
    //                                       sizeof(mbi),
    //                                       &out_size);
    //if (NT_SUCCESS(status) && mbi.State == MEM_COMMIT)
    //    return true;
    if (MmIsAddressValid(address))
        return true;
    return false;
}

bool acquire_lock(uint64_t pid, uint64_t dest)
{
    Process p(pid);
    if (p.open())
    {
        AttachProcess attach(&p);
        if (attach.fix())
        {
            if (is_address_valid((void *)dest))
            {
                if (0 == _InterlockedCompareExchange((volatile LONG *)dest, 1, 0))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
