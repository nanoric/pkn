#pragma once

#include "hash.h"
namespace compile_time
{
    class hashed
    {
    private:
        hash_t _val;
    public:
        template <class T>
        constexpr hashed(T &rhs) : _val(hash(rhs)) {}
    public:
        bool operator == (hashed rhs) const noexcept { return this->_val == rhs._val; }
        bool operator < (hashed rhs) const noexcept { return this->_val < rhs._val; }
    };
}