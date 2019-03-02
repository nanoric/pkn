#include <ntdef.h>
#include <ntifs.h>

#include <pkn/core/marcos/debug_print.h>

#include "system.h"

NTSTATUS query_system_information(UINT64 informaiton_class, void *buffer, UINT64 buffer_size, ULONG *ret_size)
{
    return ZwQuerySystemInformation(informaiton_class, buffer, (ULONG)buffer_size, ret_size);
}