/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2016 - 2017
*
*  TITLE:       MAIN.C
*
*  VERSION:     1.01
*
*  DATE:        20 Apr 2017
*
*  "Driverless" example #2
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/
#include <ntddk.h>

#include <pkn/core/base/types.h>
#include <pkn/core/registry/KernelRegistry.hpp>

#include <pkn/core/marcos/debug_print.h>

#include "names.h"
#include "protect.h"

#include "kernel_entry.h"
#include "utils/system_proc.h"
#include "global.h"

#include "dummy/dummy.h"


/************************************************************************/
/* External Function Declarations
/************************************************************************/

NTSTATUS IoDispatch(
    struct _DEVICE_OBJECT *DeviceObject,
    struct _IRP *Irp
);
NTSTATUS Mouse_Create(IN PDRIVER_OBJECT driverObject);
NTSTATUS Mouse_Close(IN PDRIVER_OBJECT driverObject);
NTSTATUS Keyboard_Create(IN PDRIVER_OBJECT driverObject);
NTSTATUS Keyboard_Close(IN PDRIVER_OBJECT driverObject);

/************************************************************************/
/* Functions
/************************************************************************/

NTSTATUS DevioctlDispatch(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
)
{
    return IoDispatch(DeviceObject, Irp);
}


NTSTATUS UnsupportedDispatch(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

NTSTATUS CreateDispatch(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    DebugPrint("%s Create", __FUNCTION__);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

NTSTATUS CloseDispatch(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    DebugPrint("%s Close", __FUNCTION__);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

char loaded_by_mmap = 0;

NTSTATUS DriverInitialize(
    _In_  struct _DRIVER_OBJECT *DriverObject,
    _In_  PUNICODE_STRING RegistryPath
)
{
    NTSTATUS        status;
    UNICODE_STRING  SymLink, DevName;
    PDEVICE_OBJECT  devobj;
    ULONG           t;

    auto g = Global::instance();

    //RegistryPath is NULL
    UNREFERENCED_PARAMETER(RegistryPath);
    DebugPrint("%s\n", __FUNCTION__);
    auto wdevice_path = g->device_path().to_wstring();
    RtlInitUnicodeString(&DevName, wdevice_path.c_str());
    DebugPrint("DevicePath : %ls", wdevice_path.c_str());
    status = IoCreateDevice(DriverObject, 0, &DevName, FILE_DEVICE_UNKNOWN, FILE_REMOVABLE_MEDIA, false, &devobj);

    DebugPrint("%s IoCreateDevice(%wZ) = %lx\n", __FUNCTION__, DevName, status);

    if (!NT_SUCCESS(status))
    {
        return status;
    }
    auto wdevice_symbol_path = g->device_symbol_path().to_wstring();
    RtlInitUnicodeString(&SymLink, wdevice_symbol_path.c_str());
    status = IoCreateSymbolicLink(&SymLink, &DevName);

    DebugPrint("%s IoCreateSymbolicLink(%wZ) = %lx\n", __FUNCTION__, SymLink, status);

    devobj->Flags |= DO_BUFFERED_IO;

    for (t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
        DriverObject->MajorFunction[t] = &UnsupportedDispatch;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DevioctlDispatch;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = &CreateDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = &CloseDispatch;
    DriverObject->DriverUnload = loaded_by_mmap ? NULL : &DriverUnload; //nonstandard way of driver loading, no unload

    //// Mouse & Keyboard Hook
    //Mouse_Create(DriverObject);
    //Keyboard_Create(DriverObject);

    //// Process Protecting
    //process_protect_install();

    //// test code
    //dummy();

    if (!Global::instance()->init())
    {
        DebugPrint("Global::init() failed, stopping setup driver");
        status = STATUS_NOT_SUPPORTED;
        return status;
    }

    devobj->Flags &= ~DO_DEVICE_INITIALIZING;
    return status;
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    process_protect_uninstall();
    IoDeleteDevice(DriverObject->DeviceObject);
    Mouse_Close(DriverObject);
    Global::release();
}


NTSTATUS mmap_stager()
{
    auto g = Global::instance();
    auto wdriver_path = g->driver_path().to_wstring();
    UNICODE_STRING  drvName;
    RtlInitUnicodeString(&drvName, wdriver_path.c_str());
    DebugPrint("MMap DriverName : %ls", wdriver_path.c_str());
    NTSTATUS status = IoCreateDriver(&drvName, &DriverInitialize);
    return status;
}

NTSTATUS DriverEntry(
    _In_  struct _DRIVER_OBJECT *DriverObject,
    _In_  PUNICODE_STRING RegistryPath
)
{
    NTSTATUS        status = STATUS_SUCCESS;

    /* This parameters are invalid due to nonstandard way of loading and should not be used. */
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        return STATUS_UNSUCCESSFUL;
    }

    auto mode = ExGetPreviousMode();
    if (mode == KernelMode)
    {
        DebugPrint("Currently : KernelMode");
    }
    else
    {
        DebugPrint("Currently : Not KernelMode");
    }

    auto g = Global::instance();
    DebugPrint("default driver name : %ls", g->driver_name.to_wstring().c_str());
    DebugPrint("default device name : %ls", g->device_name.to_wstring().c_str());
    if (RegistryPath != nullptr)
    {
        DebugPrint("Registry Path : %wZ, len:%d", RegistryPath, (int)RegistryPath->Length);

        KernelRegistry reg(estr_t(RegistryPath->Buffer, 0));
        if (!reg.open())
        {
            DebugPrint("Failed to open registry key!");
        }
        else
        {
            if (auto res = reg.get<estr_t>(L"DriverName"); res)
            {
                g->driver_name = *res;
                DebugPrint("Get DriverName From Registry : %ls", g->driver_name.to_wstring().c_str());
            }
            if (auto res = reg.get<estr_t>(L"DeviceName"); res)
            {
                g->device_name = *res;
                DebugPrint("Get DeviceName From Registry : %ls", g->device_name.to_wstring().c_str());
            }
        }
    }
    DebugPrint("CurrentDevicePath: %ls", g->device_path().to_wstring().c_str());
    DebugPrint("CurrentDeviceSymbolPath: %ls", g->device_symbol_path().to_wstring().c_str());

    if (DriverObject == nullptr && nullptr == RegistryPath)
    {
        loaded_by_mmap = 1;
        status = mmap_stager();
    }
    else
    {
        loaded_by_mmap = 0;

        // Ldr fix : force signature
        PLDR_DATA_TABLE_ENTRY ldr;
        ldr = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
        ldr->Flags |= 0x20;

        DriverInitialize(DriverObject, RegistryPath);
    }

    return status;
}

