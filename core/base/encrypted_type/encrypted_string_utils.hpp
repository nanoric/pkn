#pragma once

#include "../compile_time/const_hash.hpp"

#include "encrypted_string.hpp"
#include "encrypted_string_view.hpp"


template <class char_t>
bool operator == (const basic_encrypted_string<char_t> &lhs, const const_hash &rhs)
{
    return compile_time::run_time::hash(lhs) == rhs.value;
}

