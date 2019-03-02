#pragma once

extern "C"
{
    _Must_inspect_result_
        _IRQL_requires_max_(APC_LEVEL)
        NTKERNELAPI
        NTSTATUS
        PsLookupProcessByProcessId(
            _In_ HANDLE ProcessId,
            _Outptr_ PEPROCESS *Process
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        ZwQueryInformationProcess(
            IN  HANDLE ProcessHandle,
            IN  PROCESSINFOCLASS ProcessInformationClass,
            OUT PVOID ProcessInformation,
            IN  ULONG ProcessInformationLength,
            IN  PULONG ReturnLength
        );

    _IRQL_requires_max_(APC_LEVEL)
        NTKERNELAPI
        VOID
        KeAttachProcess(
            _Inout_ PRKPROCESS Process
        );

    _IRQL_requires_max_(APC_LEVEL)
        NTKERNELAPI
        VOID
        KeDetachProcess(
            VOID
        );

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

    NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);
}

typedef NTSTATUS(NTAPI * PUSER_THREAD_START_ROUTINE)(_In_ PVOID ThreadParameter);



typedef 
NTSTATUS
(NTAPI *fRtlCreateUserThread)(
    IN HANDLE Process,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN ULONG ZeroBits OPTIONAL,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN PVOID Parameter OPTIONAL,
    OUT PHANDLE Thread OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL
);
NTSTATUS NTAPI
RtlCreateUserThread(
    IN HANDLE Process,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN ULONG ZeroBits OPTIONAL,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN PVOID Parameter OPTIONAL,
    OUT PHANDLE Thread OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL
);


NTSTATUS get_process_base(UINT64 processid, UINT64 *base);

NTSTATUS query_process_information(UINT64 pid, UINT64 informaiton_class, void * buffer, UINT64 buffer_size, ULONG *ret_size);

NTSTATUS get_process_basic_information(UINT64 pid, PROCESS_BASIC_INFORMATION * ppbi);

NTSTATUS get_process_exit_status(UINT64 pid, NTSTATUS *pprocess_status);

NTSTATUS wait_for_process(UINT64 pid, UINT64 timeout_nanosec, NTSTATUS *result);

NTSTATUS get_process_times(UINT64 pid, UINT64 *pcreation_time, UINT64 *pexit_time, UINT64 *pkernel_time, UINT64 *puser_time);

NTSTATUS query_process_name(UINT64 pid, wchar_t * process_name, SIZE_T size_process_name);

NTSTATUS create_user_thread(IN UINT64 pid, IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL, IN BOOLEAN CreateSuspended, IN SIZE_T MaximumStackSize OPTIONAL, IN SIZE_T CommittedStackSize OPTIONAL, IN PVOID StartAddress, IN PVOID Parameter OPTIONAL, OUT PCLIENT_ID ClientId OPTIONAL);
