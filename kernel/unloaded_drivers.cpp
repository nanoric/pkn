#include <ntdef.h>
#include <ntifs.h>

#include <string>
#include <string_view>

#include <pkn/core/marcos/debug_print.h>

#include "utils/irq_guard.hpp"
#include "global.h"

#include "unloaded_drivers.h"

void move_rest_unloaded_driver(ULONG i, ULONG n, PUNLOADED_DRIVERS punloaded_drivers)
{
    for (ULONG j = i; j < n - 1; j++)
    {
        punloaded_drivers[j] = punloaded_drivers[j + 1];
    }
    //memmove(&punloaded_drivers[i], &punloaded_drivers[i + 1], n - 1 - i);
}

/*
Remove any drivers that has a name_pattern in PUNLOADED_DRIVERS->Name.
*/
NTSTATUS delete_unloaded_drivers(UINT64 rva_mm_unloaded_drivers, UINT64 rva_mm_last_unloaeded_driver, wchar_t *name_pattern, UINT64 *pout_ndeleted)
{
    // 
    if (Global::instance()->ntoskrnl_base == 0)
    {
        DebugPrint("ntoskrnl_base is zero");
        return STATUS_NOT_SUPPORTED;
    }

    // this operation is dangerous, check parameters before process
    auto g = Global::instance();
    uint64_t nt_base = g->ntoskrnl_base;
    uint64_t nt_end = nt_base + g->ntoskrnl_size;
    uint64_t p1 = nt_base + rva_mm_unloaded_drivers;
    uint64_t p2 = nt_base + rva_mm_last_unloaeded_driver;
    DebugPrint("kernel start : %llx", nt_base);
    DebugPrint("kernel end : %llx", nt_end);
    DebugPrint("p1: %llx", p1);
    DebugPrint("p2: %llx", p2);
    if (p1 < nt_base || p2 < nt_base
        || p1 >= nt_end || p2 >= nt_end)
    {
        DebugPrint("delete_unloaded_drivers(): Address is not inside ntoskrnl image!");
        return STATUS_INVALID_PARAMETER;
    }
    if (!MmIsAddressValid(reinterpret_cast<void*>(p1))
        || !MmIsAddressValid(reinterpret_cast<void*>(p2))
        )
    {
        DebugPrint("Adelete_unloaded_drivers(): ddress is not valid!");
        return STATUS_INVALID_PARAMETER;
    }

    DebugPrint("trying to delete unloaded driver : %S", name_pattern);
    auto unloaded_drivers = *(PUNLOADED_DRIVERS *)(Global::instance()->ntoskrnl_base + rva_mm_unloaded_drivers);
    auto pn = (ULONG *)(Global::instance()->ntoskrnl_base + rva_mm_last_unloaeded_driver);
    auto n = *pn;
    if (nullptr != unloaded_drivers && n > 0)
    {
        DebugPrint("nDriver : %u", (uint32_t)n);
        for (ULONG i = 0; i < n; i++)
        {
            DebugPrint("Driver : %wZ", unloaded_drivers[i].Name);
        }

        ULONG ndeleted = 0;
        std::wstring_view pattern(name_pattern);
        {
            irq_guard l(DISPATCH_LEVEL);
            for (ULONG i = 0; i < n; i++)
            {
                std::wstring_view name(unloaded_drivers[i].Name.Buffer, unloaded_drivers[i].Name.Length / sizeof(unloaded_drivers[i].Name.Buffer[0]));
                if (name.npos != name.find(name_pattern))
                {
                    move_rest_unloaded_driver(i, n, unloaded_drivers);
                    unloaded_drivers[n - 1] = { 0 };

                    ++ndeleted;
                    --n;
                    --i;
                }
            }
            *pn -= ndeleted;
        }
        *pout_ndeleted = ndeleted;

        DebugPrint("After Deleted, nDriver : %u", (uint32_t)*pn);
        for (ULONG i = 0; i < *pn; i++)
        {
            DebugPrint("Driver : %wZ", unloaded_drivers[i].Name);
        }
    }
    return STATUS_SUCCESS;
}