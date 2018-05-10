#pragma once

#include <stdint.h>
#include <string>

// if string_view isn't exist or basic_string_view not exist in std namespace
// you have to enable c++17
#include <string_view>

#include "../compile_time/hash.h"
#include "../compile_time/utils.h"
#include "encrypted_number.h"

template <class T>
class basic_encrypted_string_view : public std::basic_string_view<const_encrypted_number<T>>
{
public:
    using basic_t = T;
    using ebasic_t = const_encrypted_number<T>;
    using real_t = std::basic_string_view<basic_t>;
    using real_runtime_t = std::basic_string<basic_t>;
    using internal_t = std::basic_string_view<ebasic_t>;
public:
    using internal_t::basic_string_view;
    real_t value() const
    {
        return real_t(this->begin(), this->end());
    }
    inline operator real_t()const
    {
        return value();
    }
    inline operator real_runtime_t()const
    {
        return real_runtime_t(this->begin(), this->end());
    }
    inline std::string to_string()
    {
        return std::string(this->begin(), this->end());
    }
    inline std::wstring to_wstring()
    {
        return std::wstring(this->begin(), this->end());
    }
    inline std::u32string to_u32string()
    {
        return std::u32string(this->begin(), this->end());
    }
};

template <compile_time::random_t seed, class basic_t, basic_t ... vals >
class const_encrypted_string : public basic_encrypted_string_view<basic_t>
{
public:
    using ebasic_t = const_encrypted_number<basic_t>;
public:
    static constexpr const ebasic_t value[sizeof ... (vals)+1] = { ebasic_t(vals, seed) ..., ebasic_t(0, compile_time::random_from_seed(seed)) };
public:
    constexpr const_encrypted_string() : basic_encrypted_string_view<basic_t>(value, sizeof ... (vals)) {};
};

template <compile_time::random_t seed, class string_value_type>
class const_encstr_builder
{
public:
    template <class indexes_t>
    struct helper
    {};
    template <int ... idx>
    struct helper<indexes<idx ...>>
    {
        //using ebasic_t = typename string_value_type::ebasic_t;
        using basic_t = typename string_value_type::basic_t;
        using result = typename const_encrypted_string < seed, basic_t, string_value_type{}.value[idx] ... > ;
    };
    using result = typename helper < typename indexes_builder < string_value_type{}.size > ::result > ::result;
};

namespace compile_time
{
    template <class T>
    hash_t hash(const basic_encrypted_string_view<T> &view) noexcept
    {
        return hash(view.begin(), view.end());
    }

    template <compile_time::random_t seed, class basic_t, basic_t ... vals >
    inline constexpr hash_t hash(const const_encrypted_string<seed, basic_t, vals ...> &view) noexcept
    {
        return hash(view.begin(), view.end());
    }
}


#define make_const_encrypted_string(string_literal)                                                                                  \
[](){                                                                                                                      \
    struct str_value_type {                                                                                                \
        using basic_t = char_type_of_string_literal<decltype(string_literal)>::result;                                     \
        /*using ebasic_t = encrypted_number<char32_t>;*/                                                                        \
        const int size = sizeof(string_literal) / sizeof(basic_t) - 1;                                                         \
        const basic_t *value = string_literal;                                                                             \
    };                                                                                                                     \
    return const_encstr_builder<__COUNTER__, str_value_type>::result{};                                                                 \
}()
