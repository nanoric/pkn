#pragma once

#include "WindowsStructure.h"
#include <pkn/core/base/compile_time/hash.hpp>

namespace pkn
{
class PEUtils {
public:
    static inline uint64_t get_ppeb()
    {

#ifdef _AMD64_
        return __readgsqword(0x60);
#else
#ifdef _X86_
        return __readfsdword(0x30);
#else _ARM64_
        return (_PPEB)*(DWORD *)((BYTE *)_MoveFromCoprocessor(15, 0, 13, 0, 2) + 0x30);
#endif
#endif
    }
    static inline uint64_t get_kernel32_base(uint64_t peb)
    {

        auto ldr = (PEB_LDR_DATA64 *)(((PEB64*)peb)->Ldr);
        auto list = ldr->InMemoryOrderModuleList.Flink;

        while (list)
        {
            auto pmodule = (LDR_DATA_TABLE_ENTRY64 *)list;
            auto &dll_name = pmodule->BaseDllName;
            if (compile_time::run_time::hashstri((wchar_t *)dll_name.Buffer) == compile_time::hashi(L"Kernel32.dll"))
                return (uint64_t)pmodule->DllBase;
            list = ((LIST_ENTRY64 *)list)->Flink;
        }
        return 0;
    }

    static inline uint64_t get_get_proc_address()
    {
        uint64_t kernel32 = get_kernel32_base(get_ppeb());
        const uint64_t image_base = kernel32;

        auto mz = (PIMAGE_DOS_HEADER)image_base;
        auto pe = (PIMAGE_NT_HEADERS)(image_base + mz->e_lfanew);

        // get the VA of the modules NT Header
        auto exports = (PIMAGE_EXPORT_DIRECTORY)(image_base + pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

        auto names_number = exports->NumberOfNames;
        auto names = (uint32_t *)(image_base + exports->AddressOfNames);
        auto function_indexes = (uint16_t *)(image_base + exports->AddressOfNameOrdinals);
        auto functions = (uint32_t *)(image_base + exports->AddressOfFunctions);

        for (size_t i = 0; i < names_number; i++)
        {
            auto name = (char *)(image_base + names[i]);
            if (compile_time::run_time::hashstri(name) == compile_time::hashi("GetProcAddress"))
            {
                return image_base + functions[function_indexes[i]];
            }
        }
        return 0;
    }
};
}