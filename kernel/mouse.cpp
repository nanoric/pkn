#include "shared.h"
#include <stdio.h>

#include <pkn/core/marcos/debug_print.h>

#include "mouse.h"

#define Tag		'MOUSE'
#define MOUSECLASS_0			0x1
#define MOUSECLASS_1			0x2
#define MOUSECLASS_2			0x4
#define ALLOC_SIZE				0x1000

MouseServiceDpc MouseDpcRoutine = NULL;
mouinput MouseInputRoutine = NULL;
char mouse_pressed[5];
UINT32 last_x = 0;
UINT32 last_y = 0;
ULONG mouId = 0;


static PVOID g_node = 0;

static PDEVICE_OBJECT	g_MouseDeviceObject = NULL;
static PDEVICE_OBJECT	g_IOMouseDeviceObject = NULL;

static PDEVICE_OBJECT	g_TopOfStack;
static IRPMJREAD		g_OldReadFunction;
static IRPMJREAD		g_OldInternalDeviceFunction;
ULONGLONG *g_mouse_rootine = NULL;


static int mouse_inited = 0;

NTSTATUS Mouse_Create(IN PDRIVER_OBJECT driverObject)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING uniKbdDeviceName;
    //PDEVICE_OBJECT devicePtr;
    MouseAddDevice MouseAddDevicePtr;
    ULONGLONG node = 0;

    memset((void*)mouse_pressed, 0, sizeof(mouse_pressed));

    RtlInitUnicodeString(&uniKbdDeviceName, L"\\Device\\IOMouse");
    status = IoCreateDevice(driverObject, 0, &uniKbdDeviceName, FILE_DEVICE_MOUSE, FILE_DEVICE_SECURE_OPEN, FALSE, &g_IOMouseDeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    g_IOMouseDeviceObject->Flags |= DO_BUFFERED_IO;
    g_IOMouseDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;


    status = Mouse_Hook(driverObject);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(g_IOMouseDeviceObject);
        g_IOMouseDeviceObject = NULL;
        return status;
    }

    FindDevNodeRecurse(g_MouseDeviceObject, &node);
    if (g_IOMouseDeviceObject->DeviceObjectExtension) {
        g_node = g_IOMouseDeviceObject->DeviceObjectExtension->DeviceNode;
        g_IOMouseDeviceObject->DeviceObjectExtension->DeviceNode = (PVOID)node;
    }

    if (g_MouseDeviceObject->DriverObject) {

        if (g_MouseDeviceObject->DriverObject->DriverExtension) {
            MouseAddDevicePtr = (MouseAddDevice)g_MouseDeviceObject->DriverObject->DriverExtension->AddDevice;

            if (MouseAddDevicePtr) {
                //MouseAddDevicePtr(g_MouseDeviceObject->DriverObject, g_IOMouseDeviceObject);
            }
        }
    }
    DebugPrint("Mouse_Create ----> status[%x].\n", status);
    mouse_inited = 1;

    return status;

}
NTSTATUS Mouse_Close(IN PDRIVER_OBJECT driverObject)
{
    NTSTATUS status = STATUS_SUCCESS;

    DebugPrint("Running Mouse_Close.");
    if (mouse_inited == 0) return status;

    Mouse_UnHook(driverObject);

    if (g_IOMouseDeviceObject) {
        if (g_IOMouseDeviceObject->DeviceObjectExtension) g_IOMouseDeviceObject->DeviceObjectExtension->DeviceNode = g_node;
        IoDeleteDevice(g_IOMouseDeviceObject);
        g_IOMouseDeviceObject = NULL;
    }

    DebugPrint("Mouse_Close. Succeed");
    return status;

}

NTSTATUS Mouse_Hook(IN PDRIVER_OBJECT driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
    //ULONG i;
    int found = 0;
    NTSTATUS status = STATUS_SUCCESS;
    //PIO_STACK_LOCATION stack;
    UNICODE_STRING uniMouseDeviceName = { 0 };
    PFILE_OBJECT mouseFileObject = NULL;
    PDEVICE_OBJECT MouseDeviceObject = NULL;
    ULONG counter;
    HANDLE hDir;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING uniOa;
    PVOID pBuffer;
    PVOID pContext;
    ULONG RetLen;
    POBJECT_DIRECTORY_INFORMATION pDirBasicInfo;
    UNICODE_STRING uniMouseDrv;
    //char* mouseclsNum;
    char arMouseCls[0x10];
    WCHAR tmpNameBuffer[512];

    DebugPrint("Running Mouse_Create.");


    RtlInitUnicodeString(&uniOa, L"\\Device");

    InitializeObjectAttributes(&oa, &uniOa, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenDirectoryObject(&hDir, DIRECTORY_ALL_ACCESS, &oa);
    if (!NT_SUCCESS(status)) return status;

    pBuffer = ExAllocatePool(PagedPool, ALLOC_SIZE);
    pContext = ExAllocatePool(PagedPool, ALLOC_SIZE);
    // 		pBuffer = ExAllocatePoolWithTag(PagedPool, ALLOC_SIZE, Tag);
    // 		pContext = ExAllocatePoolWithTag(PagedPool, ALLOC_SIZE, Tag);
    memset(pBuffer, 0, ALLOC_SIZE);
    memset(pContext, 0, ALLOC_SIZE);
    memset(arMouseCls, 0, 0x10);
    counter = 0;

    while (TRUE) {
        status = ZwQueryDirectoryObject(hDir, pBuffer, ALLOC_SIZE, TRUE, FALSE, (PULONG)pContext, &RetLen);
        if (!NT_SUCCESS(status)) break;

        pDirBasicInfo = (POBJECT_DIRECTORY_INFORMATION)pBuffer;
        if (pDirBasicInfo->ObjectName.Buffer[0] == 'P')
            DebugPrint("\\Device\\%wZ", pDirBasicInfo->ObjectName);

        pDirBasicInfo->ObjectName.Length -= 2;
        RtlInitUnicodeString(&uniMouseDrv, L"PointerClass");
        if (RtlCompareUnicodeString(&pDirBasicInfo->ObjectName, &uniMouseDrv, TRUE) == 0) {
            mouId = *(char *)((char *)pDirBasicInfo->ObjectName.Buffer + pDirBasicInfo->ObjectName.Length) - '0';
            RtlInitUnicodeString(&uniMouseDeviceName, pDirBasicInfo->ObjectName.Buffer);
            DebugPrint("IDNAME: %S mouId:%d\n", uniMouseDeviceName.Buffer, mouId);
            DebugPrint("mouId:%d\n", mouId);
            found = 1;
            break;
        }
    }
    ExFreePool(pBuffer);
    ExFreePool(pContext);
    ZwClose(hDir);

    if (found == 0) {
        DebugPrint("Device finder failed.\n");
        return status;
    }

    DebugPrint("Device finder sucess!\n");
    DebugPrint("Mouse_Hook ----> mouId[0x%X].\n", mouId);

    swprintf(tmpNameBuffer, L"\\Device\\%s", uniMouseDeviceName.Buffer);
    DebugPrint("MouseDriverObject : %S", uniMouseDeviceName.Buffer);
    RtlInitUnicodeString(&uniMouseDeviceName, tmpNameBuffer);

    status = My_IoGetDeviceObjectPointer(&uniMouseDeviceName, FILE_SPECIAL_ACCESS, &mouseFileObject, &MouseDeviceObject);
    if (!NT_SUCCESS(status))
    {
        DebugPrint("My_IoGetDeviceObjectPointer Failed :%x", status);
        return status;
    }

    g_MouseDeviceObject = MouseDeviceObject;
    g_OldReadFunction = g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ];
    g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ] = Mouse_HookProc;
    g_OldInternalDeviceFunction = g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL];
    g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = Mouse_IO_InternalIoctl;
    if (NULL != mouseFileObject)
        ObDereferenceObject(mouseFileObject);

    DebugPrint("Mouse_Hook success.\n");
    return status;
}
NTSTATUS Mouse_UnHook(IN PDRIVER_OBJECT driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
    NTSTATUS status = STATUS_SUCCESS;

    DebugPrint("Running Mouse_UnHook.");
    if (g_mouse_rootine) *g_mouse_rootine = (ULONGLONG)MouseInputRoutine;
    if (g_OldReadFunction) {
        g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ] = g_OldReadFunction;
        g_OldReadFunction = NULL;
    }
    if (g_OldInternalDeviceFunction) {
        g_MouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = g_OldInternalDeviceFunction;
        g_OldInternalDeviceFunction = NULL;
    }

    DebugPrint("Mouse_UnHook. Succeed");
    return status;
}

NTSTATUS Mouse_HookProc(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    DebugPrint("Mouse_HookProc");
    NTSTATUS status;

    //PIO_APC_ROUTINE *routine = &Irp->Overlay.AsynchronousParameters.UserApcRoutine;

    //if (!MouseInputRoutine)
    //{
    //    MouseInputRoutine = (mouinput)*routine;
    //}

    //*routine = MouseApc;
    //g_mouse_rootine = routine;

    PMOUSE_INPUT_DATA mouIrp = (PMOUSE_INPUT_DATA)Irp->UserBuffer;
    if (mouIrp->ButtonFlags&MOUSE_LEFT_BUTTON_DOWN)
    {
        mouse_pressed[0] = 1;
    }
    else if (mouIrp->ButtonFlags&MOUSE_LEFT_BUTTON_UP)
    {
        mouse_pressed[0] = 0;
    }
    else if (mouIrp->ButtonFlags&MOUSE_RIGHT_BUTTON_DOWN)
    {
        mouse_pressed[1] = 1;
    }
    else if (mouIrp->ButtonFlags&MOUSE_RIGHT_BUTTON_UP)
    {
        mouse_pressed[1] = 0;
    }
    else if (mouIrp->ButtonFlags&MOUSE_MIDDLE_BUTTON_DOWN)
    {
        mouse_pressed[2] = 1;
    }
    else if (mouIrp->ButtonFlags&MOUSE_MIDDLE_BUTTON_UP)
    {
        mouse_pressed[2] = 0;
    }
    else if (mouIrp->ButtonFlags&MOUSE_BUTTON_4_DOWN)
    {
        mouse_pressed[3] = 1;
    }
    else if (mouIrp->ButtonFlags&MOUSE_BUTTON_4_UP)
    {
        mouse_pressed[3] = 0;
    }
    else if (mouIrp->ButtonFlags&MOUSE_BUTTON_5_DOWN)
    {
        mouse_pressed[4] = 1;
    }
    else if (mouIrp->ButtonFlags&MOUSE_BUTTON_5_UP)
    {
        mouse_pressed[4] = 0;
    }
    DebugPrint("MouseApc : %u, %x, %x, (%d, %d) %d,%d", mouIrp->UnitId, mouIrp->ButtonFlags, mouIrp->Flags, mouIrp->LastX, mouIrp->LastY, (int)mouse_pressed[0], (int)mouse_pressed[1]);
    if (mouIrp->Flags & MOUSE_MOVE_ABSOLUTE)
    {
        last_x = mouIrp->LastX;
        last_y = mouIrp->LastY;
    }
    else
    {
        last_x += mouIrp->LastX;
        last_y += mouIrp->LastY;
    }


    status = g_OldReadFunction(DeviceObject, Irp);
    return status;
}
NTSTATUS Mouse_IO_InternalIoctl(PDEVICE_OBJECT device, PIRP irp)
{
    UNREFERENCED_PARAMETER(device);
    PIO_STACK_LOCATION ios;
    PCONNECT_DATA cd;
    DebugPrint("Mouse_IO_InternalIoctl");

    ios = IoGetCurrentIrpStackLocation(irp);

    if (ios->Parameters.DeviceIoControl.IoControlCode == MOUCLASS_CONNECT_REQUEST)
    {
        cd =(PCONNECT_DATA) ios->Parameters.DeviceIoControl.Type3InputBuffer;

        MouseDpcRoutine = (MouseServiceDpc)cd->ClassService;
        DebugPrint("Mouse_IO_InternalIoctl ----> MouseDpcRoutine[%x]\n", MouseDpcRoutine);
    }

    //return g_OldInternalDeviceFunction(device, irp);

    return STATUS_SUCCESS;
}


ULONG get_mouse_state(UINT32 key)
{
    if (mouse_pressed[key]) return 1;

    return 0;
}
void synthesize_mouse(PMOUSE_INPUT_DATA mouse_data, SIZE_T count, ULONG fill)
{
    KIRQL irql;

    mouse_data->UnitId = (USHORT)mouId;

    KeRaiseIrql(DISPATCH_LEVEL, &irql);

    if (MouseDpcRoutine)
    {
        MouseDpcRoutine(g_MouseDeviceObject, mouse_data, mouse_data + count, &fill);
        DebugPrint("SynthesizeMouse!");
    }
    else
    {
        DebugPrint("MouseDpcRoutine == nullptr!");
    }


    KeLowerIrql(irql);
}

void get_mouse_pos(UINT32 * x, UINT32 * y)
{
    *x = last_x;
    *y = last_y;
    return;
    //OBJECT_ATTRIBUTES o;
    //UNICODE_STRING s;
    //RtlInitUnicodeString(&s, L"\\driver\\mouclass");
    //InitializeObjectAttributes(&o, &s, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    //HANDLE fileHandle = NULL;
    //NTSTATUS status;
    //{
    //    IO_STATUS_BLOCK isb = { 0 };
    //    status = ZwCreateFile(&fileHandle,
    //        FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE,
    //        &o,
    //        &isb,
    //        NULL,
    //        FILE_ATTRIBUTE_NORMAL,
    //        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    //        FILE_OVERWRITE_IF,
    //        FILE_RANDOM_ACCESS,
    //        NULL,
    //        0
    //    );
    //}
    //if (NT_SUCCESS(status)) {
    //    IO_STATUS_BLOCK isb;
    //    MOUSE_INPUT_DATA mid;
    //    LARGE_INTEGER li = { 0 };
    //    status = ZwReadFile(fileHandle, NULL, NULL, NULL, &isb, &mid, sizeof(mid), &li, NULL);
    //    if (status == STATUS_SUCCESS)
    //    {
    //        *x = mid.LastX;
    //        *y = mid.LastY;
    //    }
    //    else
    //    {
    //        *x = 0xfffe;
    //        *y = 0xfffe;
    //    }
    //    //*x = *(UINT32*)&mid;
    //    //*y = *(UINT32*)((char *)&mid + 4);
    //    ZwClose(fileHandle);
    //}
    //else
    //{
    //    *x = ~0ul;
    //    *y = ~0ul;
    //}
}
