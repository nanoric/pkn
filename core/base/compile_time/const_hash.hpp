#pragma once

#include "../compile_time/hash.hpp"

class const_hash
{
public:
    explicit constexpr const_hash(compile_time::hash_t hash)
        :
        value(hash)
    {}
    operator compile_time::hash_t()
    {
        return this->value;
    }
public:
    compile_time::hash_t value;
};
