#pragma once

#include <stdint.h>
#include "../compile_time/hash.hpp"
#include "encrypted_number.hpp"

#include "../../../stl/string"
#include "../../../stl/string_view"

template <class T>
class basic_encrypted_string : public stl::basic_string<encrypted_number<T>>
{
public:
    using basic_t = T;
    using this_t = basic_encrypted_string<basic_t>;
    using real_t = stl::basic_string<basic_t>;

    using ebasic_t = encrypted_number<basic_t>;
    using internal_t = stl::basic_string<ebasic_t>;
public:
    inline basic_encrypted_string()
    {}

    template <class IteratorType>
    inline basic_encrypted_string(IteratorType _beg, IteratorType _end)
    {
        this->reserve(_end - _beg);
        stl::copy(_beg, _end, stl::back_inserter(*this));
    }

    template <class AnyStringType>
    inline basic_encrypted_string(const AnyStringType &rhs)
    {
        //this->reserve(100);
        this->resize(100, 0); // error in kernel mode

        //this->reserve(rhs.size());
        //stl::copy(rhs.begin(), rhs.end(), stl::back_inserter(*this));
    }

    //template <class AnyType>
    //inline basic_encrypted_string(const stl::basic_string<AnyType> &rhs)
    //{
    //    stl::copy(rhs.begin(), rhs.end(), stl::back_inserter(*this));
    //}

    //template <class AnyType>
    //inline basic_encrypted_string(const stl::basic_string_view<AnyType> &rhs)
    //{
    //    this->reserve(rhs.size());
    //    stl::copy(rhs.begin(), rhs.end(), stl::back_insert_iterator(*this));
    //}

    template <class AnyCharType>
    inline basic_encrypted_string(const AnyCharType *rhs, size_t size=0)
    {
        this->reserve(size);
        stl::copy(rhs, rhs+size, stl::back_insert_iterator(*this));
    }

    template <class AnyType>
    inline bool operator == (const stl::basic_string_view<AnyType> &rhs) const
    {
        return *this == basic_encrypted_string<T>(rhs);
    }

public:
    template <class string_type>
    inline string_type to() const
    {
        string_type s;
        s.reserve(this->size());
        stl::copy(this->begin(), this->end(), stl::back_inserter(s));
        return s;
    }
    inline stl::string to_string() const
    {
        return to<stl::string>();
    }
    inline stl::wstring to_wstring() const
    {
        return to<stl::wstring>();
    }
    inline stl::u32string to_u32string() const
    {
        return to<stl::u32string>();
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

namespace eastl
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
