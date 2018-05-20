#pragma once

#include "WindowsStructure.h"
#include <core/base/compile_time/hash.h>

namespace pkn
{
    class PEUtils
    {
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
        static inline uint64_t get_kernel32_base(_PPEB peb)
        {

            auto list = peb->pLdr->InMemoryOrderModuleList.Flink;

            while (list)
            {
                auto pmodule = (PLDR_DATA_TABLE_ENTRY)list;
                auto &dll_name = pmodule->BaseDllName;
                if (compile_time::run_time::hashstri(dll_name.pBuffer) == compile_time::hashi(L"Kernel32.dll"))
                    return (uint64_t)pmodule->DllBase;
                list = list->Flink;
            }
            return 0;
        }

        static inline uint64_t get_get_proc_address()
        {
            uint64_t kernel32 = get_kernel32_base();
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

    class PEStructure
    {
    public:
        PEStructure(const void *pe_data)
            : base((uint8_t *)pe_data),
            mz((PIMAGE_DOS_HEADER)base),
            pe((PIMAGE_NT_HEADERS)(base + mz->e_lfanew))
        {}
        bool is_32bit()
        {
            return (pe->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC);
        }
        bool is_64bit()
        {
            return (pe->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);
        }
    protected:
        uint8_t * base;
        PIMAGE_DOS_HEADER mz;
        PIMAGE_NT_HEADERS pe;
    };

    struct ImportByName
    {
        const char *name;
    };

    struct ImportByOrdinal
    {
        uint64_t ordinal;
    };

    struct ImportData
    {
        union __Data
        {
            ImportByName by_name;
            ImportByOrdinal by_ordinal;
        }u;
        bool by_name;
        bool delayed;
    };

    class PEStructureDetail : PEStructure
    {
    public:
        using PEStructure::PEStructure;
    public:
        uint64_t va_to_fileoffset(uint64_t va)
        {
            auto psections = (PIMAGE_SECTION_HEADER)((uint8_t *)pe + pe->FileHeader.SizeOfOptionalHeader);
            if (psections->VirtualAddress > va)
                return va;
            for (int i = 0; i < pe->FileHeader.NumberOfSections; i++)
            {
                auto vbase = psections[i].VirtualAddress;
                auto vsize = psections[i].Misc.VirtualSize;
                auto raw_base = psections[i].PointerToRawData;
                auto raw_size = psections[i].SizeOfRawData;
                if (va >= vbase && va <= vbase + vsize)
                {
                    return va - vbase + raw_base;
                }
            }
            return 0;
        }
        uint64_t image_size()
        {
            return pe->OptionalHeader.SizeOfImage;
        }
        PIMAGE_NT_HEADERS32 pe = (PIMAGE_NT_HEADERS32)PEStructure::pe;
    };

    class PEStructure64 : PEStructure
    {
    public:
        using PEStructure::PEStructure;
    public:
        uint64_t rva_to_fileoffset(uint64_t rva)
        {
            auto psections = (PIMAGE_SECTION_HEADER)((uint8_t *)&pe->OptionalHeader + pe->FileHeader.SizeOfOptionalHeader);
            if (psections->VirtualAddress > rva)
                return rva;
            for (int i = 0; i < pe->FileHeader.NumberOfSections; i++)
            {
                auto vbase = psections[i].VirtualAddress;
                auto vsize = psections[i].Misc.VirtualSize;
                auto raw_base = psections[i].PointerToRawData;
                auto raw_size = psections[i].SizeOfRawData;
                if (rva >= vbase && rva <= vbase + vsize)
                {
                    return rva - vbase + raw_base;
                }
            }
            return 0;
        }
        uint64_t image_size()
        {
            return pe->OptionalHeader.SizeOfImage;
        }
        void parse()
        {
            // save all sections
            auto psections = (PIMAGE_SECTION_HEADER)((uint8_t *)&pe->OptionalHeader + pe->FileHeader.SizeOfOptionalHeader);
            for (int i = 0; i < pe->FileHeader.NumberOfSections; i++)
                sections.push_back(psections[i]);

            // save all imports
            auto pimports = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva_to_fileoffset(pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));
            auto n = pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
            for (int i = 0; i < n; i++)
            {
                auto &imp = pimports[i];
                if (!imp.Name)
                    break;
                std::string dll_name = (char*)base + rva_to_fileoffset(imp.Name);
                auto pthunk = (IMAGE_THUNK_DATA *)(base + rva_to_fileoffset(imp.OriginalFirstThunk ? imp.OriginalFirstThunk : imp.FirstThunk));
                while (pthunk->u1.AddressOfData)
                {
                    ImportData data;
                    if (pthunk->u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
                    {
                        data.by_name = false;
                        data.u.by_ordinal.ordinal = (uint64_t)IMAGE_ORDINAL64(pthunk->u1.Ordinal);
                        this->imports[dll_name].push_back(data);
                    }
                    else
                    {
                        data.by_name = true;
                        data.u.by_name.name = (const char *)(((PIMAGE_IMPORT_BY_NAME)(base + rva_to_fileoffset(pthunk->u1.ForwarderString)))->Name);
                        this->imports[dll_name].push_back(data);
                    }
                    pthunk++;
                }
            }
        }
        void load_as_image(void *vbase)
        {
            auto rbase = base;

            // copy header data
            memcpy(vbase, base, pe->OptionalHeader.SizeOfHeaders);

            // copy sections
            for (const auto &section : sections)
            {
                auto va = (uint8_t *)vbase + section.VirtualAddress;
                auto praw = rbase + section.PointerToRawData;
                memcpy(va, praw, section.SizeOfRawData);
            }
        }
        std::unordered_map<std::string, std::vector<ImportData>> imports;
        std::vector<IMAGE_SECTION_HEADER> sections;
        PIMAGE_NT_HEADERS64 pe = (PIMAGE_NT_HEADERS64)PEStructure::pe;
    };
}
