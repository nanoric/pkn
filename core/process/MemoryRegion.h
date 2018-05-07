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

        bool writable() const
        {
            return protect == PAGE_WRITECOPY
                || protect == PAGE_WRITECOMBINE
                || protect == PAGE_READWRITE
                || protect == PAGE_EXECUTE_WRITECOPY
                || protect == PAGE_EXECUTE_READWRITE;
        }
    public:
        inline erptr_t end() const
        {
            return base + size - 1;
        }
        inline bool readable() const
        {
            return protect == PAGE_READONLY
                || protect == PAGE_READWRITE
                || protect == PAGE_EXECUTE_READ
                || protect == PAGE_EXECUTE_READWRITE;
        }
        inline bool executable() const
        {
            return protect == PAGE_EXECUTE
                || protect == PAGE_EXECUTE_READ
                || protect == PAGE_EXECUTE_WRITECOPY
                || protect == PAGE_EXECUTE_READWRITE;
        }
        inline bool inside(erptr_t address) const
        {
            return base <= address && address <= base + size;
        }
        inline bool valid() const
        {
            return base != 0;
        }
        inline bool is_image() const
        {
            return type == MEM_IMAGE;
        }
    public:
        // used for std::upper_bound or std::lower_bound
        inline bool operator < (erptr_t address) const
        {
            return this->base < address;
        }
        inline bool operator ==(const MemoryRegion &rhs) const
        {
            return this->base == rhs.base;
        }

    public:
        static const MemoryRegion Invalid;
    };

    using MemoryRegions = std::vector<MemoryRegion>;
}
