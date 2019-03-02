#include <ntdef.h>
#include <ntifs.h>

#include <pkn/core/marcos/debug_print.h>

#include "thread.h"

ZwQueryInformationThreadT pZwQueryInformationThread = nullptr;
NTSTATUS query_thread_information(UINT64 tid, UINT64 informaiton_class, void *buffer, UINT64 buffer_size, ULONG *ret_size)
{
    if (pZwQueryInformationThread == nullptr)
    {
        UNICODE_STRING routine_string;
        RtlInitUnicodeString(&routine_string, L"ZwQueryInformationThread");
        pZwQueryInformationThread = (ZwQueryInformationThreadT)MmGetSystemRoutineAddress(&routine_string);
        if (!pZwQueryInformationThread)
            return STATUS_UNSUCCESSFUL;
    }
    NTSTATUS status;
    PETHREAD pethread;
    status = PsLookupThreadByThreadId((HANDLE)tid, &pethread);
    if (NT_SUCCESS(status))
    {
        HANDLE hthread;
        status = ObOpenObjectByPointer(pethread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsThreadType, KernelMode, &hthread);
        if (NT_SUCCESS(status))
        {
            status = pZwQueryInformationThread(hthread, (THREADINFOCLASS)informaiton_class, buffer, (ULONG)buffer_size, ret_size);
            ZwClose(hthread);
        }
        ObDereferenceObject(pethread);
    }
    return status;
}

NTSTATUS wait_for_thread(UINT64 tid, UINT64 timeout_nanosec, NTSTATUS *result)
{
    NTSTATUS status;
    PETHREAD pethread;
    status = PsLookupThreadByThreadId((HANDLE)tid, &pethread);
    if (NT_SUCCESS(status))
    {
        HANDLE hthread;
        status = ObOpenObjectByPointer(pethread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsThreadType, KernelMode, &hthread);
        if (NT_SUCCESS(status))
        {
            LARGE_INTEGER li;
            li.QuadPart = timeout_nanosec;
            *result = ZwWaitForSingleObject(hthread, FALSE, timeout_nanosec == -1 ? nullptr : &li);
            ZwClose(hthread);
        }
        else {
            DebugPrint("%s ObOpenObjectByPointer failed : %x", __FUNCTION__, status);
        }
        ObDereferenceObject(pethread);
    }
    else {
            DebugPrint("%s PsLookupThreadByThreadId %llx failed : %x", __FUNCTION__, tid, status);
    }
    return status;
}

//
//NTSTATUS suspend_thread(UINT64 tid, ULONG *previous_suspend_count)
//{
//    NTSTATUS status;
//    PETHREAD pethread;
//    status = PsLookupThreadByThreadId((HANDLE)tid, &pethread);
//    if (NT_SUCCESS(status))
//    {
//        HANDLE hthread;
//        status = ObOpenObjectByPointer(pethread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsThreadType, KernelMode, &hthread);
//        if (NT_SUCCESS(status))
//        {
//            status = PsSuspendThread(hthread, previous_suspend_count);
//            ZwClose(hthread);
//        }
//        ObDereferenceObject(pethread);
//    }
//    return status;
//}
//
//NTSTATUS resume_thread(UINT64 tid, ULONG *previous_suspend_count)
//{
//    NTSTATUS status;
//    PETHREAD pethread;
//    status = PsLookupThreadByThreadId((HANDLE)tid, &pethread);
//    if (NT_SUCCESS(status))
//    {
//        HANDLE hthread;
//        status = ObOpenObjectByPointer(pethread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsThreadType, KernelMode, &hthread);
//        if (NT_SUCCESS(status))
//        {
//            status = PsResumeThread(hthread, previous_suspend_count);
//            ZwClose(hthread);
//        }
//        ObDereferenceObject(pethread);
//    }
//    return status;
//}
//
//NTSTATUS set_thread_context(UINT64 tid, CONTEXT &context)
//{
//    NTSTATUS status;
//    PETHREAD pethread;
//    status = PsLookupThreadByThreadId((HANDLE)tid, &pethread);
//    if (NT_SUCCESS(status))
//    {
//        HANDLE hthread;
//        status = ObOpenObjectByPointer(pethread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsThreadType, KernelMode, &hthread);
//        if (NT_SUCCESS(status))
//        {
//            status = PsSetContextThread(hthread, &context, UserMode);
//            ZwClose(hthread);
//        }
//        ObDereferenceObject(pethread);
//    }
//    return status;
//}
//
//NTSTATUS get_thread_context(UINT64 tid, CONTEXT *context)
//{
//    NTSTATUS status;
//    PETHREAD pethread;
//    status = PsLookupThreadByThreadId((HANDLE)tid, &pethread);
//    if (NT_SUCCESS(status))
//    {
//        HANDLE hthread;
//        status = ObOpenObjectByPointer(pethread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *PsThreadType, KernelMode, &hthread);
//        if (NT_SUCCESS(status))
//        {
//            status = PsGetContextThread(hthread, context, UserMode);
//            ZwClose(hthread);
//        }
//        ObDereferenceObject(pethread);
//    }
//    return status;
//}
