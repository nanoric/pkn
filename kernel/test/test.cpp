#include <ntdef.h>
#include <ntifs.h>

#include <intrin.h>

#include "../marco/debug_print.h"

struct GdtRegister
{
    UINT64 base;
    UINT32 limit;
};

extern "C" UINT64 get_fs();
extern "C" UINT64 get_gs();
extern "C" UINT64 set_fs(); // mov fs, rcx(arg0)

void test()
{
    GdtRegister gdtr;
    _sgdt(&gdtr);

    DebugPrint("Gdt.base : %llx   .limit: %llx", gdtr.base, gdtr.limit);

    auto fs = get_fs();
    auto gs = get_gs();
    DebugPrint("fs : %llx   gs : %llx", fs, gs);

    // read all gdt entries
    read_physical_memory(ADDRESS, size, buffer);

    // get fs entry
    UINT64 fs_base = ...;
    read_physical_memory(ADDRESS, size, buffer);


    // edit base of fs entry
    UINT64 fs_new_base = fa_base + 0x20;
    write_physical_memory();

    // reload GDT
    _lgdt(&gdtr);

    // reload fs
    set_fs(fs);

    // test if change works.


}