#pragma once
#include "io_code.h"

typedef struct _UNLOADED_DRIVERS {
    UNICODE_STRING Name;
    PVOID StartAddress;
    PVOID EndAddress;
    LARGE_INTEGER CurrentTime;
} UNLOADED_DRIVERS, *PUNLOADED_DRIVERS;

NTSTATUS delete_unloaded_drivers(UINT64 rva_mm_unloaded_drivers, UINT64 rva_mm_last_unloaeded_driver, wchar_t * name_pattern, UINT64 *pout_ndeleted);
