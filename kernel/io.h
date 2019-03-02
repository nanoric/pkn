#pragma once

NTSTATUS IoDispatch(
    struct _DEVICE_OBJECT *DeviceObject,
    struct _IRP *Irp
);