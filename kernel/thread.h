#pragma once

NTSTATUS NTAPI PsSuspendThread(
    _In_		HANDLE ThreadHandle,
    _Out_opt_	PULONG PreviousSuspendCount
);

NTSTATUS NTAPI PsResumeThread(
    _In_		HANDLE ThreadHandle,
    _Out_opt_	PULONG PreviousSuspendCount
);


NTSTATUS NTAPI PsSetContextThread(
    _In_        HANDLE ThreadHandle,
    _In_        PCONTEXT ThreadContext,
    __in KPROCESSOR_MODE Mode
);

NTSTATUS NTAPI PsGetContextThread(
    _In_        HANDLE ThreadHandle,
    _Inout_     PCONTEXT ThreadContext,
    __in KPROCESSOR_MODE Mode
);


NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationThread(
    __in HANDLE ThreadHandle,
    __in THREADINFOCLASS ThreadInformationClass,
    __out_bcount(ThreadInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength,
    __out_opt PULONG ReturnLength
);


typedef 
NTSYSAPI
NTSTATUS
(
NTAPI
*ZwQueryInformationThreadT)(
    __in HANDLE ThreadHandle,
    __in THREADINFOCLASS ThreadInformationClass,
    __out_bcount(ThreadInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength,
    __out_opt PULONG ReturnLength
);




NTSTATUS query_thread_information(UINT64 tid, UINT64 informaiton_class, void * buffer, UINT64 buffer_size, ULONG *ret_size);

NTSTATUS suspend_thread(UINT64 tid, ULONG * previous_suspend_count);

NTSTATUS resume_thread(UINT64 tid, ULONG * previous_suspend_count);

NTSTATUS set_thread_context(UINT64 tid, CONTEXT & context);

NTSTATUS get_thread_context(UINT64 tid, CONTEXT * context);

NTSTATUS wait_for_thread(UINT64 tid, UINT64 timeout_nanosec, NTSTATUS *result);
