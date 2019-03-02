#include <ntifs.h>
#include <wdm.h>
#include <intrin.h>

#include <vector>

#include <pkn/core/marcos/debug_print.h>

#include "utils/spin_lock.hpp"
#include "protect.h"

//#include "../asm/disable_branch.hpp"

#pragma warning(push)
#pragma warning(disable : 4245)
#include "beaengine/BeaEngine.h"
#pragma warning(pop)

#define FILTER_KERNEL_ACCESS TRUE

#define MAX_PROCESS_TO_PROTECT 100

PVOID ob_callback_handle = NULL;

static spin_lock &get_data_lock()
{
    static spin_lock data_lock;
    return data_lock;
}
volatile bool protect_installed = false;
volatile bool protecting;
volatile UINT64 protecting_pid = 0;
volatile UINT64 owner_pid = 0;
volatile UINT64 owner_peprocess = 0;
volatile UINT64 protecting_peprocess = 0;
volatile UINT64 protecting_number = 0;

OB_PREOP_CALLBACK_STATUS
pre_operation_callback(
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo
);

VOID
post_operation_callback(
    _In_ PVOID RegistrationContext,
    _In_ POB_POST_OPERATION_INFORMATION PostInfo
);



//
//struct CallInfo
//{
//    asm_ptr_t entry;
//    asm_ptr_t target;
//};
//
//std::vector<CallInfo> seek_all_calls_for_function_impl(asm_ptr_t start, int max_offset_to_seek = 0x100, int max_depth = 1, int offset_seeked=0, int current_depth=0)
//{
//    UNREFERENCED_PARAMETER(max_offset_to_seek);
//    UNREFERENCED_PARAMETER(max_depth);
//    UNREFERENCED_PARAMETER(offset_seeked);
//    UNREFERENCED_PARAMETER(current_depth);
//    std::vector<CallInfo> calls;
//    for (asm_ptr_t p = start, e = start + max_offset_to_seek; p < e; p)
//    {
//        asm_pbyte_t bytes = (asm_pbyte_t)p;
//        if (bytes[0] == 0xE8) // call rel32
//        {
//            auto imidiate_val = *(int32_t *)&bytes[1];
//            auto call_target = p + 5 + imidiate_val;
//            calls.push_back(CallInfo{p, call_target});
//            p += 5;
//        }
//        else
//        {
//            DISASM d;
//            d.EIP = p;
//            int n = Disasm(&d);
//            p += n;
//        }
//    }
//    return calls;
//}
//
//std::vector<CallInfo> seek_all_calls_for_function(asm_ptr_t start, int max_offset_to_seek = 0x100, int max_depth = 1)
//{
//    return seek_all_calls_for_function_impl(start, max_offset_to_seek, max_depth);
//}
//
//std::vector<asm_ptr_t> seek_all_calls(asm_ptr_t start, int max_offset_to_seek = 0x100)
//{
//    std::vector<asm_ptr_t> calls;
//    for (asm_ptr_t p = start, e = start + max_offset_to_seek; p < e; p++)
//    {
//        asm_pbyte_t bytes = (asm_pbyte_t)p;
//        if (bytes[0] == 0xE8) // call rel32
//        {
//            auto imidiate_val = *(int32_t *)&bytes[1];
//            auto call_target = p + 5 + imidiate_val;
//            calls.push_back(call_target);
//        }
//    }
//    return calls;
//}
//
//asm_ptr_t seek_first_call_target(asm_ptr_t start, int max_offset_to_seek = 0x100)
//{
//    auto res = seek_all_calls(start, max_offset_to_seek);
//    if (res.empty())
//        return 0;
//    return res[0];
//}
//
//asm_ptr_t seek_MmVerifyCallbackFunctionCheckFlags()
//{
//    asm_ptr_t lpKeRegisterBoundCallback = get_system_proc(L"KeRegisterBoundCallback");
//    asm_ptr_t lpObRegisterCallbacks = get_system_proc(L"ObRegisterCallbacks");
//    asm_ptr_t lpMmVerifyCallbackFunction = seek_first_call_target(lpKeRegisterBoundCallback);
//    asm_ptr_t lpMmVerifyCallbackFunctionCheckFlags1 = seek_first_call_target(lpMmVerifyCallbackFunction);
//
//    auto lpMmVerifyCallbackFunctionCheckFlagses = seek_all_calls(lpObRegisterCallbacks, 0x200);
//    for (auto c : lpMmVerifyCallbackFunctionCheckFlagses)
//    {
//        if (c == lpMmVerifyCallbackFunctionCheckFlags1)
//            return c;
//    }
//    return 0;
//}
//
//asm_ptr_t seek_access_deny_entry(asm_ptr_t function_entry)
//{
//    int max_offset_to_seek = 0x300;
//    for (asm_ptr_t p = function_entry, e = function_entry + max_offset_to_seek; p < e; p++)
//    {
//        asm_pbyte_t bytes = (asm_pbyte_t)p;
//        if (bytes[0] == 0xb8 || bytes[0] == 0xb9 || bytes[0] == 0xba || bytes[0] == 0xbb) // mov e?x, ????????
//        {
//            auto imidiate_val = *(uint32_t *)&bytes[1];
//            if (imidiate_val == 0xC0000022) // mov eax, 0xC0000022
//            {
//                return p;
//            }
//        }
//    }
//    return 0;
//}
//

//
//class HookAsDirectReturn1 : public guarded_fixer
//{
//public:
//    HookAsDirectReturn1(asm_ptr_t address) : address((void *)address)
//    {
//        typed_copy<uint64_t>(org_code, this->address);
//    }
//    virtual ~HookAsDirectReturn1() override {}
//public:
//    virtual void do_fix() override
//    {
//        irql = KeRaiseIrqlToDpcLevel();
//        page_protect.do_fix();
//        typed_copy<uint64_t>(address, fix_code);
//        page_protect.do_unfix();
//        KeLowerIrql(irql);
//    }
//    virtual void do_unfix() override
//    {
//        irql = KeRaiseIrqlToDpcLevel();
//        page_protect.do_fix();
//        typed_copy<uint64_t>(address, org_code);
//        page_protect.do_unfix();
//        KeLowerIrql(irql);
//    }
//private:
//    KIRQL irql;
//    DisablePageProtect page_protect;
//    void *address = 0;
//    asm_byte_t org_code[8];
//    asm_byte_t fix_code[8] = { 0x48, 0x31, 0xC0, 0x48, 0xFF, 0xC0, 0xC3 }; // xor rax, rax; inc rax; ret
//};

NTSTATUS process_protect_install()
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;


    DebugPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ObCallbackTest: DriverEntry: Callback version 0x%hx\n", ObGetFilterVersion());


    OB_CALLBACK_REGISTRATION  CBObRegistration = {};
    OB_OPERATION_REGISTRATION CBOperationRegistrations[2] = {};
    UNICODE_STRING CBAltitude = { 0 };

    // Setup the Ob Registration calls
    CBOperationRegistrations[0].ObjectType = PsProcessType;
    CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
    CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
    CBOperationRegistrations[0].PreOperation = &pre_operation_callback;
    CBOperationRegistrations[0].PostOperation = &post_operation_callback;

    CBOperationRegistrations[1].ObjectType = PsThreadType;
    CBOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_CREATE;
    CBOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
    CBOperationRegistrations[1].PreOperation = &pre_operation_callback;
    CBOperationRegistrations[1].PostOperation = &post_operation_callback;


    RtlInitUnicodeString(&CBAltitude, L"1000");

    CBObRegistration.Version = OB_FLT_REGISTRATION_VERSION;
    CBObRegistration.OperationRegistrationCount = 2;
    CBObRegistration.Altitude = CBAltitude;
    CBObRegistration.RegistrationContext = &ob_callback_handle;
    CBObRegistration.OperationRegistration = &CBOperationRegistrations[0];

    UNICODE_STRING temp = RTL_CONSTANT_STRING(L"ObRegisterCallbacks");
    //asm_ptr_t lpObRegisterCallbacks = (asm_ptr_t)MmGetSystemRoutineAddress(&temp);
    //auto lpMmVerifyCallbackFunctionCheckFlags = seek_MmVerifyCallbackFunctionCheckFlags();
    //DebugPrint("function_entry:%llx nt!MmVerifyCallbackFunctionCheckFlags:%llx \n", lpObRegisterCallbacks, lpMmVerifyCallbackFunctionCheckFlags);
    //if (lpMmVerifyCallbackFunctionCheckFlags)
    {
        //HookAsDirectReturn1 h(lpObRegisterCallbacks);
        //h.do_fix();
        //h.do_unfix();
        status = ObRegisterCallbacks(
            &CBObRegistration,
            &ob_callback_handle       // save the registration handle to remove callbacks later
        );

        if (NT_SUCCESS(status))
        {
            DebugPrint("Process Protect initialization succeed.");
            protect_installed = true;
        }
        else
        {
            DebugPrint("Process Protect initialization failed!, status : %x", status);
        }
    }
    //else
    //{
    //    return STATUS_UNSUCCESSFUL;
    //}
    return status;
}

void process_protect_uninstall()
{
    if(protect_installed)
        ObUnRegisterCallbacks(ob_callback_handle);
}


NTSTATUS protect_process(UINT64 pid)
{
    NTSTATUS status = STATUS_INTERNAL_ERROR;
    PEPROCESS peprocess;

    spin_lock_guard l(get_data_lock());
    status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
    if (NT_SUCCESS(status))
    {
        protecting_pid = pid;
        protecting_peprocess = (UINT64)peprocess;
        owner_peprocess = (UINT64)PsGetCurrentProcess();
        owner_pid = (UINT64)PsGetCurrentProcessId();
        protecting = true;
        ObDereferenceObject(peprocess);
        DebugPrint("Process Protect set success!, status : %x, pid:%ld", status, pid);
    }
    else
    {
        DebugPrint("Process Protect set failed!, status : %x", status);
    }
    return status;
}

NTSTATUS unprotecct_process()
{
    spin_lock_guard l(get_data_lock());
    protecting = false;
    return STATUS_SUCCESS;
}

bool is_protecting()
{
    return protecting;
}

bool is_process_protected(UINT64 pid)
{
    return protecting_pid == pid;
}

bool is_process_protected(PVOID object)
{
    return protecting_peprocess == (UINT64)object;
}

bool is_owning_process(UINT64 pid)
{
    return owner_pid == pid;
}

bool is_owning_process(PVOID object)
{
    return owner_peprocess == (UINT64)object;
}

void disable_access(_Inout_ POB_PRE_OPERATION_INFORMATION PreInfo)
{
    ACCESS_MASK OriginalDesiredAccess = 0;
    PACCESS_MASK DesiredAccess = NULL;
    LPCWSTR OperationName = NULL;
    switch (PreInfo->Operation) {
    case OB_OPERATION_HANDLE_CREATE:
        DesiredAccess = &PreInfo->Parameters->CreateHandleInformation.DesiredAccess;
        OriginalDesiredAccess = PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess;

        OperationName = L"Create";
        break;

    case OB_OPERATION_HANDLE_DUPLICATE:
        DesiredAccess = &PreInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
        OriginalDesiredAccess = PreInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess;

        OperationName = L"Copy";
        break;

    default:
        break;
    }

    ACCESS_MASK InitialDesiredAccess = *DesiredAccess;
    UNREFERENCED_PARAMETER(InitialDesiredAccess);  // if DebugPrint is not defined, this value will not be referenced.

    if (!PreInfo->KernelHandle || FILTER_KERNEL_ACCESS)
    {
        *DesiredAccess = 0;
    }

    DebugPrintEx(
        DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
        "ObCallbackTest: CBTdPreOperationCallback\n"
        "    Client Id:    %p:%p\n"
        "    Object:       %p\n"
        //"    Type:         %ls\n"
        "    Operation:    %ls (KernelHandle=%d)\n"
        "    OriginalDesiredAccess: 0x%x\n"
        "    DesiredAccess (in):    0x%x\n"
        "    DesiredAccess (out):   0x%x\n",
        PsGetCurrentProcessId(),
        PsGetCurrentThreadId(),
        PreInfo->Object,
        //ObjectTypeName,
        OperationName,
        PreInfo->KernelHandle,
        OriginalDesiredAccess,

        InitialDesiredAccess,
        *DesiredAccess
    );

}

void access_filter_proc(POB_PRE_OPERATION_INFORMATION PreInfo)
{
    auto calling_process = PsGetCurrentProcess();
    if (is_owning_process(calling_process))
        return;

    if (is_process_protected(calling_process))
        return;

    DebugPrintEx(
        DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
        "Filtering...");
    if (PreInfo->ObjectType == *PsProcessType)
    {
        PVOID object = PreInfo->Object;
        if (is_process_protected(object))
            disable_access(PreInfo);
    }
    else if (PreInfo->ObjectType == *PsThreadType)
    {
        UINT64 pid_of_thread = (UINT64)PsGetThreadProcessId((PETHREAD)PreInfo->Object);
        if (is_process_protected(pid_of_thread))
            disable_access(PreInfo);
    }
}

OB_PREOP_CALLBACK_STATUS
pre_operation_callback(
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo
)
{
    DebugPrintEx(
        DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
        "Callback !");
    if (is_protecting())
    {
        UNREFERENCED_PARAMETER(RegistrationContext);
        access_filter_proc(PreInfo);
    }
    return OB_PREOP_SUCCESS;
}

VOID
post_operation_callback(
    _In_ PVOID RegistrationContext,
    _In_ POB_POST_OPERATION_INFORMATION PostInfo
)
{
    UNREFERENCED_PARAMETER(RegistrationContext);
    UNREFERENCED_PARAMETER(PostInfo);
    return;
}

