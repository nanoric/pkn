#pragma once

#include <pkn/stl/unordered_map>
#include <pkn/stl/vector>
#include <pkn/stl/functional>

namespace pkn
{

class PEStructure {
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

struct ImportByName {
    const char *name;
};

struct ImportByOrdinal {
    uint64_t ordinal;
};

struct ImportData {
    union __Data {
        ImportByName by_name;
        ImportByOrdinal by_ordinal;
    }u;
    bool by_name;
    bool delayed;
    uint64_t *imported_address; // this saves address of imported function, writing **real** entry of imported function into this address will just done this import.
};

//class PEStructureDetail : PEStructure
//{
//public:
//    using PEStructure::PEStructure;
//public:
//    uint64_t va_to_fileoffset(uint64_t va)
//    {
//        auto psections = (PIMAGE_SECTION_HEADER)((uint8_t *)pe + pe->FileHeader.SizeOfOptionalHeader);
//        if (psections->VirtualAddress > va)
//            return va;
//        for (int i = 0; i < pe->FileHeader.NumberOfSections; i++)
//        {
//            auto vbase = psections[i].VirtualAddress;
//            auto vsize = psections[i].Misc.VirtualSize;
//            auto raw_base = psections[i].PointerToRawData;
//            auto raw_size = psections[i].SizeOfRawData;
//            if (va >= vbase && va <= vbase + vsize)
//            {
//                return va - vbase + raw_base;
//            }
//        }
//        return 0;
//    }
//    uint64_t image_size()
//    {
//        return pe->OptionalHeader.SizeOfImage;
//    }
//    PIMAGE_NT_HEADERS32 pe = (PIMAGE_NT_HEADERS32)PEStructure::pe;
//};

// used to parse PE File
// before call any other functions, call parse() first.
class RawPEStructure64 : PEStructure {
public:
    using PEStructure::PEStructure;
public:
    virtual uint64_t rva_to_local_offset(uint64_t rva)
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
    uint64_t entry_point_rva()
    {
        return pe->OptionalHeader.AddressOfEntryPoint;
    }
    void parse()
    {
        // save all sections
        auto psections = (PIMAGE_SECTION_HEADER)((uint8_t *)&pe->OptionalHeader + pe->FileHeader.SizeOfOptionalHeader);
        for (int i = 0; i < pe->FileHeader.NumberOfSections; i++)
            sections.push_back(psections[i]);

        // save all imports
        auto pimports = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva_to_local_offset(pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));
        auto n = pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
        for (int i = 0; i < n; i++)
        {
            auto &imp = pimports[i];
            if (!imp.Name)
                break;
            stl::string dll_name = (char*)base + rva_to_local_offset(imp.Name);
            auto pthunk = (IMAGE_THUNK_DATA *)(base + rva_to_local_offset(imp.OriginalFirstThunk ? imp.OriginalFirstThunk : imp.FirstThunk));
            auto paddrs = (IMAGE_THUNK_DATA *)(base + rva_to_local_offset(imp.FirstThunk));
            while (pthunk->u1.AddressOfData)
            {
                ImportData data;
                data.imported_address = (uint64_t *)paddrs;
                if (pthunk->u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
                {
                    data.by_name = false;
                    data.u.by_ordinal.ordinal = (uint64_t)IMAGE_ORDINAL64(pthunk->u1.Ordinal);
                    this->imports[dll_name].push_back(data);
                }
                else
                {
                    data.by_name = true;
                    data.u.by_name.name = (const char *)(((PIMAGE_IMPORT_BY_NAME)(base + rva_to_local_offset(pthunk->u1.ForwarderString)))->Name);
                    this->imports[dll_name].push_back(data);
                }
                pthunk++;
                paddrs++;
            }
        }
    }
    using import_resolve_callback_t = stl::function<uint64_t(const stl::string &dll, const char *proc)>;
    bool resolve_imports(import_resolve_callback_t resolve)
    {
        bool success = true;
        for (auto const &p : imports)
        {
            auto const &dll = p.first;
            for (auto const &data : p.second)
            {
                if (data.by_name)
                    *data.imported_address = resolve(dll, data.u.by_name.name);
                else
                    *data.imported_address = resolve(dll, (char *)data.u.by_ordinal.ordinal);
                if (*data.imported_address == 0)
                    success = false;
            }
        }
        return success;
    }
    bool relocation(uint64_t rbase)
    {
        bool success = true;
        uint64_t diff = rbase - pe->OptionalHeader.ImageBase;
#pragma pack(push, 1)
        struct IMAGE_RELOC {
            uint16_t  offset : 12;
            uint16_t  type : 4;
        };
        using PIMAGE_RELOC = IMAGE_RELOC * ;
#pragma pack(pop)
        auto &reloc_directory = pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
        auto reloc_size = reloc_directory.Size;
        auto reloc_block = (IMAGE_BASE_RELOCATION *)(base + rva_to_local_offset(reloc_directory.VirtualAddress));
        auto size = reloc_block->SizeOfBlock;
        auto block_base = base + rva_to_local_offset(reloc_block->VirtualAddress);
        for (uint8_t *p = (uint8_t *)(reloc_block + 1), *e = p + reloc_block->SizeOfBlock - 8; p < e; p += sizeof(IMAGE_RELOC))
        {
            PIMAGE_RELOC pr = (PIMAGE_RELOC)p;
            switch (pr->type)
            {
            case IMAGE_REL_BASED_ABSOLUTE:
                break;
            case IMAGE_REL_BASED_HIGH:
                *(uint16_t *)(base + rva_to_local_offset(pr->offset)) += HIWORD(diff);
                break;
            case IMAGE_REL_BASED_LOW:
                *(uint16_t *)(base + rva_to_local_offset(pr->offset)) += LOWORD(diff);
                break;
            case IMAGE_REL_BASED_HIGHLOW:
                *(uint32_t *)(base + rva_to_local_offset(pr->offset)) += uint32_t(diff);
                break;
            case IMAGE_REL_BASED_DIR64:
                *(uint64_t *)(base + rva_to_local_offset(pr->offset)) += diff;
                break;
            case IMAGE_REL_BASED_HIGHADJ:
                break;
            default:  // unknown relocation method
                success = false;
                break;
            }
        }
        return success;
    }

    // load to an memory as an image. vbase must be a writable memory region larger than image_size()
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
    stl::unordered_map<stl::string, stl::vector<ImportData>> imports;
    stl::vector<IMAGE_SECTION_HEADER> sections;
    PIMAGE_NT_HEADERS64 pe = (PIMAGE_NT_HEADERS64)PEStructure::pe;
};

// used to parse a PE Image(Sections are loaded into memory)
class ImagePEStructure64 : public RawPEStructure64 {
public:
    using RawPEStructure64::RawPEStructure64;
    virtual uint64_t rva_to_local_offset(uint64_t rva)override
    {
        return rva;
    }
};
}
