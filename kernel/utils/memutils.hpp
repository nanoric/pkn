#pragma once

#include <pkn/core/base/types.h>

// check if val inside [begin, end)
inline bool inside(rptr_t val, rptr_t begin, rptr_t end)
{
    return val >= begin && val < end;
}

template <class T>
inline void typed_copy(void *destination, const void *data)
{
    *(T *)destination = *(T *)data;
}

inline void apply_atomic(void *destination, const void *data, size_t size)
{
    if (size == 1)
    {
        typed_copy<uint8_t>(destination, data);
    }
    else if (size == 2)
    {
        typed_copy<uint16_t>(destination, data);
    }
    else if (size <= 4)
    {
        std::aligned_storage_t<4> buffer[1];
        // copy original data into buffer
        typed_copy<uint32_t>(buffer, destination);
        // apply changes in buffer
        memcpy(buffer, data, size);
        // copy buffer to target
        typed_copy<uint32_t>(destination, buffer);
    }
    else if (size <= 8)
    {
        std::aligned_storage_t<8> buffer[1];
        // copy original data into buffer
        typed_copy<uint64_t>(buffer, destination);
        // apply changes in buffer
        memcpy(buffer, data, size);
        // copy buffer to target
        typed_copy<uint64_t>(destination, buffer);
    }
}

