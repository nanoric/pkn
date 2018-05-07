#pragma once
#include <stdint.h>

struct uint128_t
{
    uint64_t low;
    uint64_t high;
    bool operator == (const uint128_t &other)const
    {
        return this->low == other.low && this->high == other.high;
    }
    bool operator != (const uint128_t &other)const
    {
        return this->low != other.low || this->high != other.high;
    }
};

using rptr_t = uint64_t;
const rptr_t rnullptr = 0;