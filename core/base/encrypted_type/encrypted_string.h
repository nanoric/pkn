#pragma once

#include <stdint.h>
#include <string>
#include <string_view>

#include "../compile_time/hash.h"
#include "encrypted_number.h"


template <class T>
class basic_encrypted_string : public std::basic_string<encrypted_number<T>>
{
public:
    using basic_t = T;
    using ebasic_t = encrypted_number<T>;
    using real_t = std::basic_string<basic_t>;
    using internal_t = std::basic_string<ebasic_t>;
public:
    inline basic_encrypted_string()
    {}

    template <class IteratorType>
    inline basic_encrypted_string(IteratorType _beg, IteratorType _end)
        : internal_t(_beg, _end)
    {
    }

    template <class AnyType>
    inline basic_encrypted_string(const std::basic_string<AnyType> &rhs)
        : internal_t(rhs.begin(), rhs.end())
    {
    }

    template <class AnyType>
    inline basic_encrypted_string(const std::basic_string_view<AnyType> &rhs)
        : internal_t(rhs.begin(), rhs.end())
    {
    }

    template <class AnyType>
    inline basic_encrypted_string(const AnyType *rhs, size_t size)
        : internal_t(rhs, rhs + size)
    {
    }

    template <class AnyType>
    inline bool operator == (const std::basic_string_view<AnyType> &rhs) const
    {
        return *this == basic_encrypted_string<T>(rhs);
    }

public:
    real_t value() const
    {
        return real_t(this->begin(), this->end());
    }
    inline operator real_t()const
    {
        return value();
    }
    inline std::string to_string() const 
    {
        return std::string(this->begin(), this->end());
    }
    inline std::wstring to_wstring() const 
    {
        return std::wstring(this->begin(), this->end());
    }
    inline std::u32string to_u32string() const 
    {
        return std::u32string(this->begin(), this->end());
    }
    inline basic_encrypted_string<T> to_lower() const
    {
        basic_encrypted_string<T> ls;
        for (const ebasic_t &ch : *this)
        {
            if (ch >= 'A' && ch <= 'Z')
                ls.push_back(ch + 'a' - 'A');
            else
                ls.push_back(ch);
        }
        return ls;
    }
    inline basic_encrypted_string<T> to_upper() const
    {
        basic_encrypted_string<T> ls;
        for (const ebasic_t &ch : *this)
        {
            if (ch >= 'a' && ch <= 'z')
                ls.push_back(ch + 'A' - 'a');
            else
                ls.push_back(ch);
        }
        return ls;
    }
};

namespace std
{
    template <class T>
    struct hash<basic_encrypted_string<T>>
    {
        using ty = basic_encrypted_string<T>;
        uint64_t operator ()(const ty &rhs) const
        {
            if (rhs.size() == 0)
                return 0;
            return compile_time::run_time::hash(&rhs.at(0), rhs.size());
        }
    };
}

namespace compile_time
{
    template <class T>
    hash_t hash(const basic_encrypted_string<T> &str) noexcept
    {
        return hash(str.begin(), str.end());
    }
}