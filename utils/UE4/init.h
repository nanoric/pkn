#pragma once

#include <pkn/core/base/types.h>

namespace UE4
{
bool initialize_unreal_serial_environment(
    const estr_t &process_name,
    const erptr_t &ppnames_rva);
}
