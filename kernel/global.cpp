
#include <ntdef.h>
#include <ntifs.h>
#include <Ntstrsafe.h>

#include <vector>

#include <pkn/core/marcos/debug_print.h>

#include "system.h"
#include "utils/system_proc.h"

#include "global.h"

Global *Global::_instance = nullptr;

bool Global::initialize_ntoskrnl()
{
    auto routine = get_system_proc(L"NtOpenFile");
    if (routine == nullptr)
    {
        DebugPrint("could not retrive address of NtOpenFile");
        return false;
    }

    ULONG bytes = 0;

    // SystemModuleInformation == 11 == 0x0B
    NTSTATUS status = ZwQuerySystemInformation(11, 0, 0, &bytes);
    UNREFERENCED_PARAMETER(status);
    if (bytes == 0)
    {
        DebugPrint("ZwQuerySystemInformation failed");
        return false;
    }

    std::vector<unsigned char> buffer(bytes);

    auto pMods = (PRTL_PROCESS_MODULES)&buffer[0];

    // SystemModuleInformation == 11 == 0x0B
    if (!NT_SUCCESS(ZwQuerySystemInformation(11, pMods, bytes, &bytes)))
    {
        DebugPrint("ZwQuerySystemInformation failed");
        return false;
    }
    PRTL_PROCESS_MODULE_INFORMATION pMod = pMods->Modules;

    for (ULONG i = 0; i < pMods->NumberOfModules; i++)
    {
        // System routine is inside module
        if (routine >= pMod[i].ImageBase &&
            routine < (PVOID)((PUCHAR)pMod[i].ImageBase + pMod[i].ImageSize))
        {
            Global::ntoskrnl_base = (UINT64)pMod[i].ImageBase;
            Global::ntoskrnl_size = pMod[i].ImageSize;
            DebugPrint("Kernel base : %llx", Global::ntoskrnl_base);
            DebugPrint("Kernel size : %llx", Global::ntoskrnl_size);
            return true;
        }
    }
    DebugPrint("Unable to locate base of ntoskrnl");
    return false;
}

bool Global::init()
{
    if (!this->initialize_ntoskrnl()) 
        return false;
    return true;
}

void Global::release()
{
    if(_instance)
        delete _instance;
}
