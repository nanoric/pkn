#pragma once

#include <ntifs.h> 
#include <ntddmou.h>

#define MOUCLASS_CONNECT_REQUEST 0x0F0203


typedef NTSTATUS(*IRPMJREAD) (IN PDEVICE_OBJECT, IN PIRP);


typedef NTSTATUS(__fastcall *mouinput)(void *a1, void *a2, void *a3, void *a4, void *a5);
typedef void(__fastcall *MouseServiceDpc)(PDEVICE_OBJECT mou, PMOUSE_INPUT_DATA a1, PMOUSE_INPUT_DATA a2, PULONG a3);
typedef NTSTATUS(__fastcall *MouseAddDevice)(PDRIVER_OBJECT a1, PDEVICE_OBJECT a2);

// Create Keyboard Hook Create.
NTSTATUS Mouse_Create(IN PDRIVER_OBJECT driverObject);
NTSTATUS Mouse_Close(IN PDRIVER_OBJECT driverObject);
NTSTATUS Mouse_Hook(IN PDRIVER_OBJECT driverObject);
NTSTATUS Mouse_UnHook(IN PDRIVER_OBJECT driverObject);


NTSTATUS Mouse_HookProc(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS Mouse_IO_InternalIoctl(PDEVICE_OBJECT device, PIRP irp);

void synthesize_mouse(PMOUSE_INPUT_DATA mouse_data, SIZE_T count, ULONG fill);
ULONG get_mouse_state(UINT32 key);
void get_mouse_pos(UINT32 * x, UINT32 * y);
