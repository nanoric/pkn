#pragma once

#include <stdint.h>
#include "../compile_time/hash.hpp"
#include "encrypted_number.hpp"

#include <string>
#include <string_view>

template <class T>
class basic_encrypted_string : public std::basic_string<encrypted_number<T>>
{
public:
    using basic_t = T;
    using this_t = basic_encrypted_string<basic_t>;
    using real_t = std::basic_string<basic_t>;

    using ebasic_t = encrypted_number<basic_t>;
    using internal_t = std::basic_string<ebasic_t>;
public:
    inline basic_encrypted_string()
    {}

    template <class IteratorType>
    inline basic_encrypted_string(IteratorType _beg, IteratorType _end)
    {
        this->reserve(_end - _beg);
        std::copy(_beg, _end, std::back_inserter(*this));
    }

    template <class AnyStringType>
    inline basic_encrypted_string(const AnyStringType &rhs)
        //: internal_t(rhs.begin(), rhs.end())
    {
        this->reserve(rhs.size());
        std::copy(rhs.begin(), rhs.end(), std::back_inserter(*this));
    }

    //template <class AnyType>
    //inline basic_encrypted_string(const std::basic_string<AnyType> &rhs)
    //{
    //    std::copy(rhs.begin(), rhs.end(), std::back_inserter(*this));
    //}

    //template <class AnyType>
    //inline basic_encrypted_string(const std::basic_string_view<AnyType> &rhs)
    //{
    //    this->reserve(rhs.size());
    //    std::copy(rhs.begin(), rhs.end(), std::back_insert_iterator(*this));
    //}

    template <class AnyCharType>
    inline basic_encrypted_string(const AnyCharType *rhs, size_t size=0)
    {
        if (size == 0)
            size = std::basic_string<AnyCharType>(rhs).size();
        this->reserve(size);
        std::copy(rhs, rhs+size, std::back_insert_iterator(*this));
    }

    template <class AnyType>
    inline bool operator == (const std::basic_string_view<AnyType> &rhs) const
    {
        return *this == basic_encrypted_string<T>(rhs);
    }

public:
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
    namespace run_time
    {
        template <class T>
        hash_t hash(const basic_encrypted_string<T> &str) noexcept
        {
            if (str.empty())
                return 0;
            return hash(&*str.begin(), str.size());
        }
    }
}
