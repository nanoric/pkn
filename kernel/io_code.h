#pragma once
#include <ntddmou.h>

#ifndef FILE_DEVICE_UNKNOWN 
#define FILE_DEVICE_UNKNOWN 0x00000022
#endif

#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED 0
#endif

#ifndef FILE_SPECIAL_ACCESS
#define  FILE_SPECIAL_ACCESS 0
#endif

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#define IOCTL_PLAYERKNOWNS_READ_PROCESS_MEMORY                      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0810, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_READ_PROCESS_MEMORIES                    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0811, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_WRITE_PROCESS_MEMORY                     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0812, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_ALLOCATE_VIRTUAL_MEMORY                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0813, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_PROTECT_VIRTUAL_MEMORY                   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0814, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_FREE_VIRTUAL_MEMORY                      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0815, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_ALLOCATE_NONPAGED_MEMORY                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0816, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_READ_SYSTEM_MEMORY                       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0817, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_WRITE_SYSTEM_MEMORY                      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0818, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_GET_PROCESS_BASE                         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0820, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_QUERY_VIRTUAL_MEMORY                     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0821, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_MAPPED_FILE                          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0822, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_PROCESS_EXIT_STATUS                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0823, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_WAIT_FOR_PROCESS                         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0824, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_PROCESS_TIMES                        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0825, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_PROCESS_NAME                         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0826, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_QUERY_PROCESS_INFORMATION                CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0827, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_CREATE_USER_THREAD                       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0828, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_WRITE_PHISICAL_MEMORY                    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0840, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_READ_PHISICAL_MEMORY                     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0841, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_PHISICAL_ADDRESS                     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0842, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_GET_MOUSE_POS                            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0850, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_MOUSE_STATE                          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0851, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_SYNTHESIZE_MOUSE                         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0852, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_GET_KEYS_STATE                           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0860, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_PROTECT_PROCESS                          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0870, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_UNPROTECT_PROCESS                        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0871, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_PLAYERKNOWNS_QUERY_THREAD_INFORMATION                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0881, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_THREAD_CONTEXT                       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0882, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_SET_THREAD_CONTEXT                       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0883, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_SUSPEND_THREAD                           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0884, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_RESUME_THREAD                            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0885, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_CREATE_THREAD                            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0885, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_GET_THREAD_EXIT_CODE                     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0885, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_WAIT_FOR_THREAD                          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0886, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_CREATE_SYSTEM_THREAD                     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0887, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_QUERY_SYSTEM_INFORMATION                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0890, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_DELETE_UNLOADED_DRIVERS                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0900, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_DIRECT_CALL                              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0910, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_RUN_DRIVER_ENTRY                         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0911, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_MAP_AND_RUN_DRIVER                       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0912, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PLAYERKNOWNS_FORCE_WRITE_PROCESS_MEMORY               CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0920, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PLAYERKNOWNS_ACQUIRE_LOCK                              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0930, METHOD_BUFFERED, FILE_ANY_ACCESS)


#pragma pack(push, 8)
///////////////////////////////////////////////////////
// memory
///////////////////////////////////////////////////////
// RPM
typedef struct __ReadProcessMemoryInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 startaddress;
    UINT64 bytestoread;
    UINT64 buffer;
}ReadProcessMemoryInput;

//// RPMs
//typedef struct __ReadProcessMemoriesInputData
//{
//    UINT64 startaddress;
//    UINT64 bytestoread;
//    UINT64 buffer;
//}ReadProcessMemoriesInputData;
//typedef struct __ReadProcessMemoriesInputHead
//{
//    UINT64 xor_val;
//    UINT64 processid;
//    UINT64 count;
//}ReadProcessMemoriesInputHead;

// WPM
typedef struct __WriteProcessMemoryInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 startaddress;
    UINT64 bytestowrite;
    UINT64 buffer;
}WriteProcessMemoryInput;

// QueryVirtualMemory
typedef struct __QueryVirtualMemoryInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 address;
}QueryVirtualMemoryInput;

typedef struct __QueryVirtualMemoryOutput
{
    MEMORY_BASIC_INFORMATION mbi;
}QueryVirtualMemoryOutput;

// AllocateVirtualMemory
typedef struct __AllocateVirtualMemoryInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 address; // can be zero
    UINT64 size;
    UINT32 type;    // MEM_COMMIT, MEM_RESERVE
    UINT32 protect;
}AllocateVirtualMemoryInput;

typedef struct __AllocateVirtualMemoryOutput
{
    UINT64 address;
    UINT64 size;
}AllocateVirtualMemoryOutput;

// FreeVirtualMemory
typedef struct __FreeVirtualMemoryInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 address; // can be zero
    UINT64 size;
    UINT32 type;    // MEM_COMMIT, MEM_RESERVE
}FreeVirtualMemoryInput;

typedef struct __FreeVirtualMemoryOutput
{
    UINT64 address;
    UINT64 size;
}FreeVirtualMemoryOutput;

// ProtectVirtualMemory
typedef struct __ProtectVirtualMemoryInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 address; // can be zero
    UINT64 size;
    UINT32 protect;
}ProtectVirtualMemoryInput;

typedef struct __ProtectVirtualMemoryOutput
{
    UINT64 address;
    UINT64 size;
    ULONG old_protect;
}ProtectVirtualMemoryOutput;

// AllocateNonpagedMemory
typedef struct __AllocateNonpagedMemoryInput
{
    UINT64 xor_val;
    UINT64 size;
}AllocateNonpagedMemoryInput;

typedef struct __AllocateNonpagedMemoryOutput
{
    UINT64 xor_val;
    UINT64 address;
}AllocateNonpagedMemoryOutput;

typedef struct __ReadSystemMemoryInput
{
    UINT64 xor_val;
    UINT64 addresstoread;
    UINT64 bytestoread;
    UINT64 buffer;
}ReadSystemMemoryInput;

typedef struct __ReadSystemMemoryOutput
{
    UINT64 xor_val;
    UINT64 bytesread;
}ReadSystemMemoryOutput;

typedef struct __WriteSystemMemoryInput
{
    UINT64 xor_val;
    UINT64 addresstowrite;
    UINT64 bytestowrite;
    UINT64 buffer;
}WriteSystemMemoryInput;

typedef struct __WriteSystemMemoryOutput
{
    UINT64 xor_val;
    UINT64 byteswritten;
}WriteSystemMemoryOutput;

///////////////////////////////////////////////////////
// process
///////////////////////////////////////////////////////
// GetProcessBase
typedef struct __GetProcessBaseInput
{
    UINT64 xor_val;
    UINT64 processid;
}GetProcessBaseInput;

typedef struct __GetProcessBaseOutput
{
    UINT64 base;
}GetProcessBaseOutput;

// GetMappedFile
typedef struct __GetMappedFileInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 address;
}GetMappedFileInput;

typedef struct __GetMappedFileOutput
{
    wchar_t image_path[512];
}GetMappedFileOutput;

// TestProcess
typedef struct __TestProcessInput
{
    UINT64 xor_val;
    UINT64 processid;
}TestProcessInput;

typedef struct __TestProcessOutput
{
    NTSTATUS status;
}TestProcessOutput;

// WaitProcess
typedef struct __WaitProcessInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 timeout_nanosec; // zero for immediate return
}WaitProcessInput;

typedef struct __WaitProcessOutput
{
    NTSTATUS status;
}WaitProcessOutput;

// GetProcessTimes
typedef struct __GetProcessTimesInput
{
    UINT64 xor_val;
    UINT64 processid;
}GetProcessTimesInput;

typedef struct __GetProcessTimesOutput
{
    UINT64 creation_time;
    UINT64 exit_time;
    UINT64 kernel_time;
    UINT64 user_time;
}GetProcessTimesOutput;

// GetProcessName
typedef struct __GetProcessNameInput
{
    UINT64 xor_val;
    UINT64 processid;
}GetProcessNameInput;

typedef struct __GetProcessNameOutput
{
    wchar_t process_name[512];
}GetProcessNameOutput;


// GetProcessName
typedef struct __CreateUserThreadInput
{
    UINT64 xor_val;
    UINT64 processid;
    SECURITY_DESCRIPTOR sd;
    bool has_sd;
    bool create_suspended;
    UINT64 maximun_stack_size;
    UINT64 commited_stack_size;
    UINT64 start_address;
    UINT64 parameter;
}CreateUserThreadInput;

typedef struct __CreateUserThreadOutput
{
    UINT64 pid; // process id
    UINT64 tid; // thread id
}CreateUserThreadOutput;





///////////////////////////////////////////////////////
// physical memory
///////////////////////////////////////////////////////
// WritePhysicalMemory
typedef struct __WritePhisicalMemoryInput
{
    UINT64 address;
    UINT64 bytestowrite;
}WritePhisicalMemoryInput;


// ReadPhysicalMemory
typedef struct __ReadPhisicalMemoryInput
{
    UINT64 address;
    UINT64 bytestoread;
}ReadPhisicalMemoryInput;

// GetPhysicalAddress
typedef struct __GetPhisicalAddressInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 address;
}GetPhisicalAddressInput;

///////////////////////////////////////////////////////
// mouse
///////////////////////////////////////////////////////
// GetMousePos
typedef struct __GetMousePosOutput
{
    UINT32 x;
    UINT32 y;
}GetMousePosOutput;

// GetMouseState
typedef struct __GetMouseStateInput
{
    UINT32 key;
}GetMouseStateInput;
typedef struct __GetMouseStateOutput
{
    UINT32 pressed;
}GetMouseStateOutput;

// SynthesizeMouse
typedef MOUSE_INPUT_DATA SynthesizeMouseInputData;
typedef struct __SynthesizeMouseInputHead
{
    UINT64 count;
    ULONG fill;
}SynthesizeMouseInputHead;

///////////////////////////////////////////////////////
// process protect
///////////////////////////////////////////////////////
typedef struct __ProtectProcessInput
{
    UINT64 xor_val;
    UINT64 pid;
}ProtectProcessInput;

///////////////////////////////////////////////////////
// thread
///////////////////////////////////////////////////////
typedef struct __QueryInformationInput
{
    UINT64 xor_val;
    UINT64 id;
    UINT64 information_class;
    ULONG buffer_size;
    bool size_only;
}QueryInformationInput;
typedef struct __QueryInformationOutput
{
    ULONG ret_size;
    char buffer[2048];
}QueryInformationOutput;

using QueryProcessInformationInput = QueryInformationInput;
//using QueryProcessInformationOutput = QueryInformationOutput;
using QueryThreadInformationInput = QueryInformationInput;
//using QueryThreadInformationOutput = QueryInformationOutput;
using QuerySystemInformationInput = QueryInformationInput;
//using QuerySystemInformationOutput = QueryInformationOutput;


// set context
typedef struct __SetThreadContextInput
{
    UINT64 xor_val;
    UINT64 tid;
    CONTEXT context;
}SetThreadContextInput;

// get context
typedef struct __GetThreadContextInput
{
    UINT64 xor_val;
    UINT64 tid;
}GetThreadContextInput;

typedef struct __GetThreadContextOutput
{
    CONTEXT context;
}GetThreadContextOutput;

// suspend thread
typedef struct __SuspendThreadInput
{
    UINT64 xor_val;
    UINT64 tid;
}SuspendThreadInput;

typedef struct __SuspendThreadOutput
{
    ULONG previois_suspend_count;
}SuspendThreadOutput;

// resume thread
typedef struct __ResumeThreadInput
{
    UINT64 xor_val;
    UINT64 tid;
}ResumeThreadInput;

typedef struct __ResumeThreadOutput
{
    ULONG previois_suspend_count;
}ResumeThreadOutput;

// wait thread
typedef struct __WaitThreadInput
{
    UINT64 xor_val;
    UINT64 tid;
    UINT64 timeout_nanosec; // zero for immediate return
}WaitThreadInput;

typedef struct __WaitThreadOutput
{
    NTSTATUS status;
}WaitThreadOutput;

//// craete system thread
//typedef struct __CreateSystemThreadInput
//{
//    UINT64 xor_val;
//    UINT64 start_address;
//    UINT64 parameter; // zero for immediate return
//}CreateSystemThreadInput;


//////////////////////////////////////////////////////////////////////////
// Utils 
//////////////////////////////////////////////////////////////////////////

// unloaded drivers
typedef struct __DeleteUnloadedDriversInput
{
    UINT64 xor_val;
    UINT64 pMmUnloadedDrivers;
    UINT64 pMmLastUnloadedDriver;
    wchar_t name[512];
}DeleteUnloadedDriversInput;

typedef struct __DeleteUnloadedDriversOutput
{
    UINT64 ndelete;
}DeleteUnloadedDriversOutput;

// direct call
typedef struct __DirectCallInput
{
    UINT64 xor_val;
    UINT64 start_address;
    UINT64 parameter; // zero for immediate return
}DirectCallInput;

typedef struct __DirectCallOutput
{
    UINT64 xor_val;
    UINT64 ret_rax;
}DirectCallOutput;

// direct call
typedef struct __RunDriverEntryInput
{
    UINT64 xor_val;
    UINT64 start_address;
    UINT64 parameter1;
    UINT64 parameter2;
}RunDriverEntryInput;

typedef struct __RunDriverEntryOutput
{
    UINT64 xor_val;
    UINT64 ret_val;
}RunDriverEntryOutput;

typedef struct __MapAndRunDriverInput
{
    UINT64 xor_val;
    UINT64 image_buffer;
    UINT64 image_size;
    UINT64 entry_rva;
    UINT64 parameter1;
    UINT64 parameter2;
    bool parameter1_is_rva;
    bool parameter2_is_rva;
}MapAndRunDriverInput;

typedef struct __MapAndRunDriverOutput
{
    UINT64 xor_val;
    UINT64 ret_val;
}MapAndRunDriverOutput;


struct AcquireLockInput
{
    UINT64 xor_val;
    UINT64 processid;
    UINT64 remote_lock_address;
};

struct AcquireLockOutput
{
    UINT64 xor_val;
    bool succeed;
};

#pragma pack(pop)
