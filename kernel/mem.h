#pragma once

#include <optional>

#include "io_code.h"

extern "C"
{

    _IRQL_requires_max_(APC_LEVEL)
        NTKERNELAPI
        VOID
        KeStackAttachProcess(
            _Inout_ PRKPROCESS PROCESS,
            _Out_ PRKAPC_STATE ApcState
        );

    _IRQL_requires_max_(APC_LEVEL)
        NTKERNELAPI
        VOID
        KeUnstackDetachProcess(
            _In_ PRKAPC_STATE ApcState
        );

    NTKERNELAPI
        NTSTATUS
        NTAPI
        MmCopyVirtualMemory(
            IN PEPROCESS FromProcess,
            IN PVOID FromAddress,
            IN PEPROCESS ToProcess,
            OUT PVOID ToAddress,
            IN SIZE_T BufferSize,
            IN KPROCESSOR_MODE PreviousMode,
            OUT PSIZE_T NumberOfBytesCopied
        );

    NTSYSAPI
        NTSTATUS NTAPI ZwAllocateVirtualMemory(
            _In_    HANDLE    ProcessHandle,
            _Inout_ PVOID     *BaseAddress,
            _In_    ULONG_PTR ZeroBits,
            _Inout_ PSIZE_T   RegionSize,
            _In_    ULONG     AllocationType,
            _In_    ULONG     Protect
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        ZwFreeVirtualMemory(
            _In_ HANDLE ProcessHandle,
            _Inout_ PVOID *BaseAddress,
            _Inout_ PSIZE_T RegionSize,
            _In_ ULONG FreeType
        );

    typedef NTSYSAPI
        NTSTATUS
        (NTAPI *fZwProtectVirtualMemory)(
            __in HANDLE ProcessHandle,
            __inout PVOID *BaseAddress,
            __inout PSIZE_T RegionSize,
            __in ULONG NewProtect,
            __out PULONG OldProtect
            );

    //NTSYSAPI
    //    NTSTATUS
    //    NTAPI
    //    ZwProtectVirtualMemory(
    //        __in HANDLE ProcessHandle,
    //        __inout PVOID *BaseAddress,
    //        __inout PSIZE_T RegionSize,
    //        __in ULONG NewProtect,
    //        __out PULONG OldProtect
    //    );


#define MAP_PROCESS 0x01
#define MAP_SYSTEM  0x02

    NTSTATUS
        ZwLockVirtualMemory(
            __in HANDLE ProcessHandle,
            __inout PVOID *BaseAddress,
            __inout PSIZE_T RegionSize,
            __in ULONG MapType
        );
    using fZwLockVirtualMemory = NTSTATUS(*)
        (
            __in HANDLE ProcessHandle,
            __inout PVOID *BaseAddress,
            __inout PSIZE_T RegionSize,
            __in ULONG MapType
            );

    NTSTATUS
        ZwUnlockVirtualMemory(
            __in HANDLE ProcessHandle,
            __inout PVOID *BaseAddress,
            __inout PSIZE_T RegionSize,
            __in ULONG MapType
        );
    using fZwUnlockVirtualMemory = NTSTATUS(*)
        (
            __in HANDLE ProcessHandle,
            __inout PVOID *BaseAddress,
            __inout PSIZE_T RegionSize,
            __in ULONG MapType
            );

}

NTSTATUS read_process_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer);

//NTSTATUS read_process_memories(UINT64 processid, SIZE_T count, const ReadProcessMemoriesInputData * datas);

NTSTATUS write_process_memory(UINT64 processid, UINT64 address, SIZE_T size, UINT64 buffer);

NTSTATUS query_virtual_memory(UINT64 pid, UINT64 address, MEMORY_BASIC_INFORMATION *pmbi);

NTSTATUS get_mapped_file(UINT64 pid, UINT64 address, wchar_t * image_path, SIZE_T size_path);

NTSTATUS allocate_virtual_memory(UINT64 pid, PVOID * BaseAddress, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);

NTSTATUS free_virtual_memory(UINT64 pid, PVOID * BaseAddress, PSIZE_T RegionSize, ULONG FreeType);

NTSTATUS protect_virtual_memory(UINT64 pid, PVOID * BaseAddress, PSIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect);

std::optional<void *> allocate_nonpaged_memory(SIZE_T size);

size_t copy_virtual_memory(void *dest, void *src, size_t size);

bool force_write_8(uint64_t pid, void * dest, uint8_t val);
bool force_write_64(uint64_t pid, void * dest, uint64_t val);
// don't copy large memory using this!!!
bool force_write(uint64_t pid, void * dest, void *src, size_t size);

bool acquire_lock(uint64_t pid, uint64_t dest);
