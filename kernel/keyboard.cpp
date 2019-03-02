#include "shared.h"
#include <stdio.h>

#include "keyboard.h"

#define DebugPrint(...) 
#define DebugPrintEx(...) 

//#define DebugPrint( ...) DbgPrint(__VA_ARGS__)
//#define DebugPrintEx( ...) DbgPrintEx(__VA_ARGS__)


#define Tag		'KYBD'
#define KBDCLASS_0			0x1
#define KBDCLASS_1			0x2
#define KBDCLASS_2			0x4
#define ALLOC_SIZE				0x1000


KeyboardServiceDpc KeyboardDpcRoutine=NULL;
kbdinput KeyboardInputRoutine=NULL;
PKEYBOARD_INPUT_DATA kbdIrp=NULL;
KEYBOARD_INPUT_DATA kdata;

static PDEVICE_OBJECT	g_KbdDeviceObject = NULL;
static PDEVICE_OBJECT g_IOKbdDeviceObject = NULL;
static PVOID g_node=0;


PDEVICE_OBJECT	g_TopOfStack;

IRPMJREAD		g_OldReadFunction = NULL;
IRPMJREAD		g_OldInternalDeviceFunction = NULL;

ULONGLONG *g_keyboard_rootine = NULL;

char KEY_DATA[256];
ULONG kbdId=0;
static int keyboard_inited = 0;


NTSTATUS Keyboard_Create(IN PDRIVER_OBJECT driverObject)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniKbdDeviceName;
	//PDEVICE_OBJECT devicePtr;
	KeyboardAddDevice KeyboardAddDevicePtr;
	ULONGLONG node=0;
	int i;

	memset((void*)&kdata,0,sizeof(kdata));
	for(i=0; i<256; i++) KEY_DATA[i]=0;

	RtlInitUnicodeString(&uniKbdDeviceName,L"\\Device\\IOKeyboard");
	status = IoCreateDevice(driverObject,0,&uniKbdDeviceName,FILE_DEVICE_UNKNOWN,FILE_DEVICE_SECURE_OPEN,FALSE,&g_IOKbdDeviceObject);
	if(!NT_SUCCESS(status)) {
		return status;
	}

	g_IOKbdDeviceObject->Flags|=DO_BUFFERED_IO; 
	g_IOKbdDeviceObject->Flags&=~DO_DEVICE_INITIALIZING;


	status = Keyboard_Hook(driverObject);
	if(!NT_SUCCESS(status)) {
		return status;
	}

	FindDevNodeRecurse(g_KbdDeviceObject,&node);

	if(g_IOKbdDeviceObject->DeviceObjectExtension) {
		g_node = g_IOKbdDeviceObject->DeviceObjectExtension->DeviceNode;
		g_IOKbdDeviceObject->DeviceObjectExtension->DeviceNode=(PVOID)node;
	}

	if(g_KbdDeviceObject->DriverObject) {

		if(g_KbdDeviceObject->DriverObject->DriverExtension) {
			KeyboardAddDevicePtr=(KeyboardAddDevice)g_KbdDeviceObject->DriverObject->DriverExtension->AddDevice;

			if(KeyboardAddDevicePtr) {
				KeyboardAddDevicePtr(g_KbdDeviceObject->DriverObject,g_IOKbdDeviceObject);
			}
		}
	}
	DebugPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Keyboard_Init ----> Keyboard_Init status[%x].\n", status);

	keyboard_inited = 1;
	return status;

}
NTSTATUS Keyboard_Close(IN PDRIVER_OBJECT driverObject)
{
	NTSTATUS status = STATUS_SUCCESS;

	if( keyboard_inited == 0 ) return status;

 	Keyboard_UnHook(driverObject);

	if(g_IOKbdDeviceObject) {
		if(g_IOKbdDeviceObject->DeviceObjectExtension) g_IOKbdDeviceObject->DeviceObjectExtension->DeviceNode = g_node;
		IoDeleteDevice(g_IOKbdDeviceObject);
		g_IOKbdDeviceObject = NULL;
	}

	keyboard_inited  = 0;

	return status;

}
NTSTATUS Keyboard_Hook(IN PDRIVER_OBJECT driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
	//ULONG i;
	int found = 0;
	NTSTATUS status = STATUS_SUCCESS;
	//PIO_STACK_LOCATION stack;
    UNICODE_STRING uniKbdDeviceName = { 0 };
	PFILE_OBJECT KbdFileObject;
	PDEVICE_OBJECT KbdDeviceObject;
	HANDLE hDir;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniOa;
	PVOID pBuffer;
	PVOID pContext;
	ULONG RetLen;
	POBJECT_DIRECTORY_INFORMATION pDirBasicInfo;
	UNICODE_STRING uniKbdDrv;
	//char* KbdclsNum;
	char arKbdCls[0x10];
	WCHAR tmpNameBuffer[512];

	DebugPrint("Running Keyboard_Create.");


	RtlInitUnicodeString(&uniOa, L"\\Device");
	InitializeObjectAttributes(&oa,	&uniOa,	OBJ_CASE_INSENSITIVE, NULL,	NULL);
	status = ZwOpenDirectoryObject(	&hDir, DIRECTORY_ALL_ACCESS, &oa );
	if(!NT_SUCCESS(status)) return status;

	pBuffer = ExAllocatePoolWithTag(PagedPool, ALLOC_SIZE, Tag);
	pContext = ExAllocatePoolWithTag(PagedPool, ALLOC_SIZE, Tag);
	memset(pBuffer, 0, ALLOC_SIZE);
	memset(pContext, 0, ALLOC_SIZE);
	memset(arKbdCls, 0, 0x10);

	while(TRUE)	{
		status = ZwQueryDirectoryObject( hDir, pBuffer, ALLOC_SIZE, TRUE, FALSE, (PULONG)pContext, &RetLen );
		if(!NT_SUCCESS(status))	break;

		pDirBasicInfo =	(POBJECT_DIRECTORY_INFORMATION)pBuffer;

		pDirBasicInfo->ObjectName.Length -= 2;

		RtlInitUnicodeString(&uniKbdDrv, L"KeyboardClass");

		if(RtlCompareUnicodeString( &pDirBasicInfo->ObjectName,	&uniKbdDrv,	FALSE ) == 0)
		{
			kbdId = (ULONG)(*(char *)(pDirBasicInfo->ObjectName.Buffer+pDirBasicInfo->ObjectName.Length));
			kbdId -= 0x30;
			pDirBasicInfo->ObjectName.Length += 2;
			RtlInitUnicodeString(&uniKbdDeviceName, pDirBasicInfo->ObjectName.Buffer);

			found = 1;
			break;
		}
		pDirBasicInfo->ObjectName.Length += 2;
	}

	ExFreePool(pBuffer);
	ExFreePool(pContext);
	ZwClose(hDir);
	if( found == 0 ) return status;
	DebugPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Keyboard_Hook ----> kbdId[%d].\n", kbdId);

	swprintf(tmpNameBuffer, L"\\Device\\%s", uniKbdDeviceName.Buffer);
	RtlInitUnicodeString(&uniKbdDeviceName, tmpNameBuffer);
	status = My_IoGetDeviceObjectPointer( &uniKbdDeviceName, FILE_ALL_ACCESS, &KbdFileObject,	&KbdDeviceObject );

	if(NT_SUCCESS(status))
	{
		g_KbdDeviceObject = KbdDeviceObject;
		ObDereferenceObject(KbdFileObject);
		g_OldReadFunction = g_KbdDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ];
		g_KbdDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ] = Keyboard_HookProc;

		g_OldInternalDeviceFunction = g_KbdDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL];
		g_KbdDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = Keyboard_IO_InternalIoctl;

		DebugPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"KeyBoard_Create ----> KeyBoard_HookProc success.\n");
	}
	return STATUS_SUCCESS;
}
NTSTATUS Keyboard_UnHook(IN PDRIVER_OBJECT driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
	//ULONG i;
	NTSTATUS status=STATUS_SUCCESS;
	//PIO_STACK_LOCATION stack;
	//UNICODE_STRING uniKbdDeviceName;
	//PFILE_OBJECT KbdFileObject;
	//PDEVICE_OBJECT KbdDeviceObject;
	//ULONG counter;
	//HANDLE hDir;
	//OBJECT_ATTRIBUTES oa;
	//UNICODE_STRING uniOa;
	//PVOID pBuffer;
	//PVOID pContext;
	//ULONG RetLen;
	//PDIRECTORY_BASIC_INFORMATION pDirBasicInfo;
	//UNICODE_STRING uniKbdDrv;
	//char* KbdclsNum;
	//char arKbdCls[0x10];

	DebugPrint("Running Keyboard_Close.");

	if( keyboard_inited == 0 ) return status;

	if(g_keyboard_rootine) *g_keyboard_rootine = (ULONGLONG)KeyboardInputRoutine;

	if(g_OldReadFunction)
	{
		g_KbdDeviceObject->DriverObject->MajorFunction[IRP_MJ_READ] = g_OldReadFunction;
		g_OldReadFunction = NULL;
	}
	if(g_OldInternalDeviceFunction)
	{
		g_KbdDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = g_OldInternalDeviceFunction;
		g_OldInternalDeviceFunction = NULL;
	}

	return STATUS_SUCCESS;
}


NTSTATUS KeyboardApc(void *a1, void *a2, void *a3, void *a4, void *a5)
{
	unsigned char max=(unsigned char)kbdIrp->MakeCode;

	if(!kbdIrp->Flags)
	{
		KEY_DATA[(max)-1]=1;
	}
	else if(kbdIrp->Flags&KEY_BREAK)
	{
		KEY_DATA[(max)-1]=0;
	}
	//DebugPrint("KeyboardApc: max=%x", max);

	return KeyboardInputRoutine(a1,a2,a3,a4,a5);
}

NTSTATUS Keyboard_HookProc( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	PIO_STACK_LOCATION ios;
	PCONNECT_DATA cd;
	NTSTATUS status;


// 	DebugPrint("KeyBoard_HookProc: success");

	ios=IoGetCurrentIrpStackLocation(Irp);
	if(ios->Parameters.DeviceIoControl.IoControlCode==KBDCLASS_CONNECT_REQUEST && KeyboardDpcRoutine == NULL)
	{
		cd=(PCONNECT_DATA)ios->Parameters.DeviceIoControl.Type3InputBuffer;

		KeyboardDpcRoutine=(KeyboardServiceDpc)cd->ClassService;
		DebugPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Keyboard_IO_InternalIoctl ----> KeyboardDpcRoutine[%x]\n", KeyboardDpcRoutine);
	}

	kbdIrp=(PKEYBOARD_INPUT_DATA)Irp->UserBuffer;
	unsigned char max=(unsigned char)kbdIrp->MakeCode;
	if(!kbdIrp->Flags)
	{
		KEY_DATA[(max)-1]=1;
	}
	else if(kbdIrp->Flags&KEY_BREAK)
	{
		KEY_DATA[(max)-1]=0;
	}

	status = g_OldReadFunction(DeviceObject, Irp);

	return status;
}
NTSTATUS Keyboard_IO_InternalIoctl(PDEVICE_OBJECT device, PIRP irp)
{
    UNREFERENCED_PARAMETER(device);
	PIO_STACK_LOCATION ios;
	PCONNECT_DATA cd;
	//NTSTATUS status;

	ios=IoGetCurrentIrpStackLocation(irp);

	if(ios->Parameters.DeviceIoControl.IoControlCode==KBDCLASS_CONNECT_REQUEST)
	{
		cd=(PCONNECT_DATA)ios->Parameters.DeviceIoControl.Type3InputBuffer;

		KeyboardDpcRoutine=(KeyboardServiceDpc)cd->ClassService;
		DebugPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Keyboard_IO_InternalIoctl ----> KeyboardDpcRoutine[%x]\n", KeyboardDpcRoutine);
	}

	//if(g_OldInternalDeviceFunction) status = g_OldInternalDeviceFunction(device, irp);

	return STATUS_SUCCESS;
}

int GetKeyState(unsigned char scan)
{
	if(KEY_DATA[scan-1]) return 1;

	return 0;
}

void SynthesizeKeyboard(PKEYBOARD_INPUT_DATA a1)
{
	KIRQL irql;
	char *endptr;
	ULONG fill=1;

	endptr=(char*)a1;

	endptr+=sizeof(KEYBOARD_INPUT_DATA);

	a1->UnitId=(USHORT)kbdId;

	KeRaiseIrql(DISPATCH_LEVEL,&irql);

	if(KeyboardDpcRoutine) KeyboardDpcRoutine(g_KbdDeviceObject,a1,(PKEYBOARD_INPUT_DATA)endptr,&fill);

	KeLowerIrql(irql);
}
