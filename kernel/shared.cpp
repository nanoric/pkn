#include <pkn/core/marcos/debug_print.h>

#include "shared.h"

NTSTATUS
My_IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
)
{
    PFILE_OBJECT fileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatus = { 0 };
    NTSTATUS status;
    InitializeObjectAttributes(&objectAttributes,
        ObjectName,
        OBJ_KERNEL_HANDLE,
        (HANDLE)NULL,
        (PSECURITY_DESCRIPTOR)NULL);
    status = ZwOpenFile(&fileHandle,
        DesiredAccess,
        &objectAttributes,
        &ioStatus,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        FILE_NON_DIRECTORY_FILE);
    if (NT_SUCCESS(status))
    {
        status = ObReferenceObjectByHandle(fileHandle,
            0,
            *IoFileObjectType,
            KernelMode,
            (PVOID *)&fileObject,
            NULL);
        if (NT_SUCCESS(status))
        {
            *FileObject = fileObject;
            *DeviceObject = IoGetBaseFileSystemDeviceObject(fileObject);
        }
        else
        {
            DebugPrint("ObReferenceObjectByHandle Failed : 0x%X", status);
        }
        ZwClose(fileHandle);
    }
    else
    {
        DebugPrint("ZwOpenFile Failed for %wZ: 0x%X", objectAttributes.ObjectName, status);
    }
    return status;
}

void FindDevNodeRecurse(PDEVICE_OBJECT a1, ULONGLONG *a2)
{
    struct DEVOBJ_EXTENSION_FIX *attachment;

    attachment = (struct DEVOBJ_EXTENSION_FIX*)a1->DeviceObjectExtension;

    if ((!attachment->AttachedTo) && (!attachment->DeviceNode)) return;

    if ((!attachment->DeviceNode) && (attachment->AttachedTo))
    {
        FindDevNodeRecurse(attachment->AttachedTo, a2);
        return;
    }

    *a2 = (ULONGLONG)attachment->DeviceNode;
    return;
}