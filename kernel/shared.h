#pragma once
#include <ntifs.h>
#include <kbdmou.h>


void FindDevNodeRecurse(PDEVICE_OBJECT a1, ULONGLONG *a2);
extern NTSTATUS My_IoGetDeviceObjectPointer(IN PUNICODE_STRING ObjectName, IN ACCESS_MASK DesiredAccess, OUT PFILE_OBJECT *FileObject, OUT PDEVICE_OBJECT *DeviceObject);
struct DEVOBJ_EXTENSION_FIX
{
    USHORT type;
    USHORT size;
    PDEVICE_OBJECT devObj;
    ULONGLONG PowerFlags;
    void *Dope;
    ULONGLONG ExtensionFlags;
    void *DeviceNode;
    PDEVICE_OBJECT AttachedTo;
};


typedef struct _DIRECTORY_BASIC_INFORMATION
{
    UNICODE_STRING ObjectName;
    UNICODE_STRING ObjectTypeName;

} OBJECT_DIRECTORY_INFORMATION,
*POBJECT_DIRECTORY_INFORMATION;

extern "C"
{
    NTSYSAPI
        NTSTATUS
        NTAPI
        ZwOpenDirectoryObject(
            OUT PHANDLE DirectoryHandle,
            IN ACCESS_MASK DesiredAccess,
            IN POBJECT_ATTRIBUTES
            ObjectAttributes
        );

    typedef NTSTATUS(NTAPI *PNTOPENDIRECTORYOBJECT)(
        OUT PHANDLE DirectoryHandle,
        IN ACCESS_MASK DesiredAccess,
        IN POBJECT_ATTRIBUTES
        ObjectAttributes
        );
    NTSYSAPI
        NTSTATUS
        NTAPI
        ZwQueryDirectoryObject(
            IN HANDLE  DirectoryHandle,
            OUT PVOID  Buffer,
            IN ULONG   BufferLength,
            IN BOOLEAN ReturnSingleEntry,
            IN BOOLEAN RestartScan,
            IN OUT PULONG  Context,
            OUT PULONG ReturnLength OPTIONAL
        );

    typedef NTSTATUS(NTAPI *PNTQUERYDIRECTORYOBJECT)(
        IN HANDLE  DirectoryHandle,
        OUT PVOID  Buffer,
        IN ULONG   BufferLength,
        IN BOOLEAN ReturnSingleEntry,
        IN BOOLEAN RestartScan,
        IN OUT PULONG  Context,
        OUT PULONG ReturnLength OPTIONAL
        );
}
