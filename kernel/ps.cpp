#include <ntdef.h>
#include <ntifs.h>

#include <pkn/core/marcos/debug_print.h>

#include "ps.h"
#include "utils\system_proc.h"

NTSTATUS get_process_base(UINT64 processid, UINT64 *base)
{
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS peprocess;

    status = PsLookupProcessByProcessId((HANDLE)processid, &peprocess);
    if (NT_SUCCESS(status))
    {
        KAPC_STATE state;
        KeStackAttachProcess((PKPROCESS)peprocess, &state);
        *base = (UINT64)PsGetProcessSectionBaseAddress(peprocess);
        KeUnstackDetachProcess(&state);
        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS query_process_information(UINT64 pid, UINT64 informaiton_class, void *buffer, UINT64 buffer_size, ULONG *ret_size)
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
            status = ZwQueryInformationProcess(hprocess, (PROCESSINFOCLASS)informaiton_class, buffer, (ULONG)buffer_size, ret_size);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS get_process_basic_information(UINT64 pid, PROCESS_BASIC_INFORMATION *ppbi)
{
    ULONG ret_size = 0;
    return query_process_information(pid, ProcessBasicInformation, ppbi, sizeof(*ppbi), &ret_size);
}

NTSTATUS get_process_exit_status(UINT64 pid, NTSTATUS *pprocess_status)
{
    NTSTATUS status;
    PROCESS_BASIC_INFORMATION pbi;
    *pprocess_status = -1;
    status = get_process_basic_information(pid, &pbi);
    if (NT_SUCCESS(status))
    {
        *pprocess_status = pbi.ExitStatus;
    }
    return status;
}


NTSTATUS get_process_peb_address(UINT64 pid, UINT64 *ppeb)
{
    NTSTATUS status;
    PROCESS_BASIC_INFORMATION pbi;
    status = get_process_basic_information(pid, &pbi);
    if (NT_SUCCESS(status))
    {
        *ppeb = (UINT64)(pbi.PebBaseAddress);
    }
    return status;
}


NTSTATUS wait_for_process(UINT64 pid, UINT64 timeout_nanosec, NTSTATUS *result)
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
            LARGE_INTEGER li;
            li.QuadPart = timeout_nanosec;
            *result = ZwWaitForSingleObject(hprocess, FALSE, timeout_nanosec == -1 ? nullptr : &li);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS get_process_times(UINT64 pid, UINT64 *pcreation_time, UINT64 *pexit_time, UINT64 *pkernel_time, UINT64 *puser_time)
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
            UINT64 times[4];
            ULONG retsize;
            status = ZwQueryInformationProcess(hprocess, (PROCESSINFOCLASS)0x04, times, sizeof(times), &retsize);
            if (NT_SUCCESS(status))
            {
                if (pcreation_time)*pcreation_time = times[0];
                if (pexit_time)*pexit_time = times[1];
                if (pkernel_time)*pkernel_time = times[2];
                if (puser_time)*puser_time = times[3];
            }
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}

NTSTATUS query_process_name(UINT64 pid, wchar_t *process_name, SIZE_T size_process_name)
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
            char buffer[1024] = { 0 };
            UNICODE_STRING *ustr = (UNICODE_STRING *)buffer;

            ULONG retsize;
            status = ZwQueryInformationProcess(hprocess, ProcessImageFileName, buffer, sizeof(buffer), &retsize);
            if (NT_SUCCESS(status))
            {
                SIZE_T ncopy = size_process_name - sizeof(*process_name) < ustr->Length ? size_process_name - sizeof(*process_name) : ustr->Length;
                RtlCopyMemory(process_name, ustr->Buffer, ustr->Length);
                process_name[ncopy / sizeof(*process_name)] = 0;
            }
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}
NTSTATUS create_user_thread(
    IN UINT64 pid,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN PVOID StartAddress,
    IN PVOID Parameter OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL
)
{

    static fRtlCreateUserThread pRtlCreateUserThread = (fRtlCreateUserThread)get_system_proc(L"RtlCreateUserThread");
    if (pRtlCreateUserThread == nullptr)
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
            HANDLE thread_handle;
            DebugPrint("RtlCreateUserThread %llx, %llx | %d, %d | %llu, %llu | 0x%llx, 0x%llx | 0x%llx, 0x%llx",
                hprocess,
                ThreadSecurityDescriptor,
                CreateSuspended,
                0,
                MaximumStackSize,
                CommittedStackSize,
                (PUSER_THREAD_START_ROUTINE)StartAddress,
                Parameter, &thread_handle, ClientId
            );
            status = pRtlCreateUserThread(hprocess, ThreadSecurityDescriptor, CreateSuspended, 0, MaximumStackSize, CommittedStackSize, (PUSER_THREAD_START_ROUTINE)StartAddress, Parameter, &thread_handle, ClientId);
            if (NT_SUCCESS(status))
                ZwClose(thread_handle);
            else
                DebugPrint("RtlCreateUserThread failed : %x", status);
            ZwClose(hprocess);
        }
        ObDereferenceObject(peprocess);
    }
    return status;
}
