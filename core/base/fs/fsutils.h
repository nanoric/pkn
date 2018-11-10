#pragma once

#include "../types.h"

inline estr_t file_name_for_path(const estr_t &path)
{
    size_t offset = path.rfind(U'\\');
    if (offset != -1)
    {
        return path.substr(offset + 1);
    }
    return path;
}
