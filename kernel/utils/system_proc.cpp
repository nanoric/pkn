#include <ntifs.h>

#include <pkn/core/marcos/debug_print.h>

#include "system_proc.h"

void *get_system_proc(wchar_t *name)
{
    UNICODE_STRING temp;
    RtlInitUnicodeString(&temp, name);
    auto retv = MmGetSystemRoutineAddress(&temp);

    if (retv) {
        DebugPrint("Find proc : %ws at %llx", name, retv);
    }
    else {
        DebugPrint("Unable to find proc : %ws", name);
    }
    return retv;
}