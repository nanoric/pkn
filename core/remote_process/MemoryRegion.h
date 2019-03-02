#pragma once

#include "disable_windows_min_max_definetion.h"
#include "../base/types.h"
namespace pkn
{
class MemoryRegion
{
public:
    erptr_t base = 0;
    encrypted_number<size_t> size = 0;
    encrypted_number<size_t> protect;
    erptr_t allocation_base;
    uint32_t type;

    bool writable() const noexcept
    {
        return protect == PAGE_WRITECOPY
            || protect == PAGE_WRITECOMBINE
            || protect == PAGE_READWRITE
            || protect == PAGE_EXECUTE_WRITECOPY
            || protect == PAGE_EXECUTE_READWRITE;
    }
public:
    inline erptr_t end() const noexcept
    {
        return base + size - 1;
    }
    inline bool readable() const noexcept
    {
        return protect == PAGE_READONLY
            || protect == PAGE_READWRITE
            || protect == PAGE_EXECUTE_READ
            || protect == PAGE_EXECUTE_READWRITE;
    }
    inline bool executable() const noexcept
    {
        return protect == PAGE_EXECUTE
            || protect == PAGE_EXECUTE_READ
            || protect == PAGE_EXECUTE_WRITECOPY
            || protect == PAGE_EXECUTE_READWRITE;
    }
    inline bool inside(erptr_t address) const noexcept
    {
        return base <= address && address <= base + size;
    }
    inline bool valid() const noexcept
    {
        return base != 0;
    }
    inline bool is_image() const noexcept
    {
        return type == MEM_IMAGE;
    }
public:
    inline bool operator < (const erptr_t &address) const noexcept
    {
        return this->base < address;
    }
    inline bool operator <(const MemoryRegion &rhs) const noexcept
    {
        return this->base < rhs.base;
    }
    inline bool operator ==(const MemoryRegion &rhs) const noexcept
    {
        return this->base == rhs.base;
    }

public:
    static const MemoryRegion Invalid;
};

using MemoryRegions = std::vector<MemoryRegion>;

// for std::upper_bound works
inline bool operator < (const erptr_t &address, const MemoryRegion &region)
{
    return address < region.base;
}
}
