#pragma once
#include "utils/spin_lock.hpp"
#include <pkn/core/base/types.h>
#include "names.h"

class Global {
public:
    UINT64 ntoskrnl_base = 0;
    UINT64 ntoskrnl_size = 0;
    estr_t driver_name = make_estr(PlayerKnownsDriverName);
    estr_t device_name = make_estr(PlayerKnownsDriverDeviceName);
    estr_t driver_path()
    {
        auto retv = estr_t(make_estr("\\Driver\\")) + this->driver_name;
        return retv;
    }
    estr_t device_path()
    {
        return estr_t(make_estr("\\Device\\")) + this->device_name;
    }
    estr_t device_symbol_path()
    {
        return estr_t(make_estr("\\DosDevices\\")) + this->device_name;
    }

    // note : there is no lock for this function.
    // user should call instance() at program startup
    static Global *instance()
    {
        if (_instance == nullptr)
        {
            _instance = new Global;
        }
        return _instance;
    }
    static void release();
public:
    bool init();
private:
    static Global *_instance;
    bool initialize_ntoskrnl();
};

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;         // Not filled in
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;
