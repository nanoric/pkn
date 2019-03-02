#include "init.h"

#include "UnrealReader.hpp"
#include "UnrealNamesCache.hpp"

#include "../Environment/Environment.hpp"

using namespace pkn;

namespace UE4
{
bool initialize_unreal_serial_environment(
    const estr_t &process_name,
    const erptr_t &ppnames_rva)
{
    Environment env(process_name);
    if (env.init())
    {
        auto base = env.process->base();
        // engine specific reader
        UnrealReader *r = new UnrealReader;
        pkn::SingletonInjector<UnrealReader>::set(r);

        // names cache
        auto ppnames = base + ppnames_rva;
        UnrealNameCache *nc = new UnrealNameCache(ppnames);
        if (!nc->init())
        {
            DebugPrint("Failed to initialize UnrealNameCache!\n");
            return false;
        }
        pkn::SingletonInjector<UnrealNameCache>::set(nc);
        return true;
    }
    return false;
}
}

