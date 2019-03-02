#pragma once


using WORD = unsigned short;
typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    WORD LoadCount;
    WORD TlsIndex;
    union
    {
        LIST_ENTRY HashLinks;
        struct __s1
        {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union
    {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    void * EntryPointActivationContext;
    PVOID PatchInformation;
    LIST_ENTRY ForwarderLinks;
    LIST_ENTRY ServiceTagLinks;
    LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

extern "C"
{
    NTKERNELAPI
        NTSTATUS
        IoCreateDriver(
            IN PUNICODE_STRING DriverName, OPTIONAL
            IN PDRIVER_INITIALIZE InitializationFunction
        );

}

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DevioctlDispatch;
_Dispatch_type_(IRP_MJ_CREATE)
DRIVER_DISPATCH CreateDispatch;
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH CloseDispatch;

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CREATE_NAMED_PIPE)
_Dispatch_type_(IRP_MJ_CLOSE)
_Dispatch_type_(IRP_MJ_READ)
_Dispatch_type_(IRP_MJ_WRITE)
_Dispatch_type_(IRP_MJ_QUERY_INFORMATION)
_Dispatch_type_(IRP_MJ_SET_INFORMATION)
_Dispatch_type_(IRP_MJ_QUERY_EA)
_Dispatch_type_(IRP_MJ_SET_EA)
_Dispatch_type_(IRP_MJ_FLUSH_BUFFERS)
_Dispatch_type_(IRP_MJ_QUERY_VOLUME_INFORMATION)
_Dispatch_type_(IRP_MJ_SET_VOLUME_INFORMATION)
_Dispatch_type_(IRP_MJ_DIRECTORY_CONTROL)
_Dispatch_type_(IRP_MJ_FILE_SYSTEM_CONTROL)
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
_Dispatch_type_(IRP_MJ_INTERNAL_DEVICE_CONTROL)
_Dispatch_type_(IRP_MJ_SHUTDOWN)
_Dispatch_type_(IRP_MJ_LOCK_CONTROL)
_Dispatch_type_(IRP_MJ_CLEANUP)
_Dispatch_type_(IRP_MJ_CREATE_MAILSLOT)
_Dispatch_type_(IRP_MJ_QUERY_SECURITY)
_Dispatch_type_(IRP_MJ_SET_SECURITY)
_Dispatch_type_(IRP_MJ_POWER)
_Dispatch_type_(IRP_MJ_SYSTEM_CONTROL)
_Dispatch_type_(IRP_MJ_DEVICE_CHANGE)
_Dispatch_type_(IRP_MJ_QUERY_QUOTA)
_Dispatch_type_(IRP_MJ_SET_QUOTA)
_Dispatch_type_(IRP_MJ_PNP)
DRIVER_DISPATCH UnsupportedDispatch;

DRIVER_INITIALIZE DriverInitialize;
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD     DriverUnload;
