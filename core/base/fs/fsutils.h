#pragma once

#include "../types.h"

namespace pkn
{
template <class T>
inline T filename_for_path(const T &path)
{
    size_t offset = path.rfind(T::value_type('\\'));
    if (offset != -1)
    {
        return path.substr(offset + 1);
    }
    return path;
}
}
