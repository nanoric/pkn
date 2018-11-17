#pragma once

#include <stdint.h>

#include <string>
#include <string_view>

#include "../compile_time/hash.hpp"
#include "../compile_time/utils.hpp"
#include "encrypted_number.hpp"

template <class char_t>
class basic_encrypted_string_view : public std::basic_string_view<const_encrypted_number<char_t>>
{
public:
    using basic_t = char_t;
    using ebasic_t = const_encrypted_number<char_t>;
    using real_t = std::basic_string_view<basic_t>;
    using real_runtime_t = std::basic_string<basic_t>;
    using internal_t = std::basic_string_view<ebasic_t>;
public:
    using internal_t::basic_string_view;
    //real_t value() const
    //{
    //    return real_t(this->begin(), this->end());
    //}
    //inline operator real_t()const
    //{
    //    return value();
    //}
    //inline operator real_runtime_t()const
    //{
    //    return real_runtime_t(this->begin(), this->end());
    //}
    template <class string_type>
    inline string_type to() const
    {
        string_type s;
        s.reserve(this->size());
        std::copy(this->begin(), this->end(), std::back_inserter(s));
        return s;
    }
    inline std::string to_string() const
    {
        return to<std::string>();
    }
    inline std::wstring to_wstring() const
    {
        return to<std::wstring>();
    }
    inline std::u32string to_u32string() const
    {
        return to<std::u32string>();
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
    template <size_t ... idx>
    struct helper<std::index_sequence<idx ...>>
    {
        //using ebasic_t = typename string_value_type::ebasic_t;
        using basic_t = typename string_value_type::basic_t;
        using result = typename const_encrypted_string < seed, basic_t, string_value_type{}.value[idx] ... > ;
    };
    using result = typename helper<typename std::make_index_sequence < string_value_type{}.size >> ::result;
};

namespace compile_time
{
    template <compile_time::random_t seed, class basic_t, basic_t ... vals >
    inline constexpr hash_t hash(const const_encrypted_string<seed, basic_t, vals ...> &view) noexcept
    {
        if constexpr (view.size() == 0)
            return 0;
        return hash(view.begin(), view.end());
    }
}

namespace compile_time
{
    namespace run_time
    {
        template <class char_t>
        hash_t hash(const basic_encrypted_string_view<char_t> &view) noexcept
        {
            if (view.length() == 0)
                return 0;
            return hash(&view[0], view.size());
        }
    }
}

#define make_const_encrypted_string(string_literal)                                               \
[](){                                                                                             \
    struct str_value_type {                                                                       \
        using basic_t = char_type_of_string_literal<decltype(string_literal)>::result;            \
        /*using ebasic_t = encrypted_number<char32_t>;*/                                          \
        const int size = sizeof(string_literal) / sizeof(basic_t) - 1;                            \
        const basic_t *value = string_literal;                                                    \
    };                                                                                            \
    return const_encstr_builder<__COUNTER__, str_value_type>::result{};                           \
}()
