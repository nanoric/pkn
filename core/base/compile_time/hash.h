#pragma once


#pragma warning(push)
#pragma warning(disable : 4307)

namespace compile_time
{
    using hash_t = uint64_t;
    constexpr const hash_t __hash_basis = 14695981039346656037ULL;
    constexpr const hash_t __hash_prime = 281474976710677ULL;

    /*
    I don't use a hash type, even though it provides more control for template
    Because :
    1. No needs for more control or information
    2. function template is easier to use
    */

    template <class T>
    inline constexpr hash_t hash(T)
    {
        static_assert(false, "compile time Hash for this type is not unavailable, declare it by yourself!");
    }

    template <class CharType, size_t size>
    inline constexpr hash_t hash(const CharType(&str)[size]) noexcept
    {
        constexpr hash_t _FNV_offset_basis = __hash_basis;
        constexpr hash_t _FNV_prime = __hash_prime;
        hash_t _Val = _FNV_offset_basis;
        for (hash_t _Next = 0; _Next < size; ++_Next)
        {
            const hash_t ch = str[_Next];
            if (ch == '\0')
                continue;
            _Val ^= (hash_t)ch;
            _Val *= _FNV_prime;
        }
        return _Val;
    }

    template <class CharType, size_t size>
    inline constexpr hash_t hashi(const CharType(&str)[size]) noexcept
    {
        constexpr hash_t _FNV_offset_basis = __hash_basis;
        constexpr hash_t _FNV_prime = __hash_prime;
        hash_t _Val = _FNV_offset_basis;
        for (hash_t _Next = 0; _Next < size; ++_Next)
        {
            const hash_t ch = str[_Next];
            if (ch == '\0')
                continue;
            const hash_t hash_v = ch >= 'a' && ch <= 'z' ? ch + 'A' - 'a' : ch;
            _Val ^= (hash_t)hash_v;
            _Val *= _FNV_prime;
        }
        return _Val;
    }

    template <class IteratorType>
    inline constexpr hash_t hash(IteratorType _begin, IteratorType _end) noexcept
    {
        constexpr hash_t _FNV_offset_basis = __hash_basis;
        constexpr hash_t _FNV_prime = __hash_prime;
        hash_t _Val = _FNV_offset_basis;
        for (_begin, _end; _begin != _end; _begin++)
        {
            const hash_t ch = *_begin;
            if (ch == 0)
                continue;
            _Val ^= (hash_t)ch;
            _Val *= _FNV_prime;
        }
        return _Val;
    }
}

namespace compile_time
{
    namespace run_time
    {
        template <class CharType>
        inline const hash_t hash(const CharType *str, size_t size) noexcept
        {
            const hash_t _FNV_offset_basis = __hash_basis;
            const hash_t _FNV_prime = __hash_prime;
            hash_t _Val = _FNV_offset_basis;
            for (hash_t _Next = 0; _Next < size; ++_Next)
            {
                const hash_t ch = str[_Next];
                if (ch == '\0')
                    continue;
                _Val ^= (hash_t)ch;
                _Val *= _FNV_prime;
            }
            return _Val;
        }

        template <class CharType>
        inline const hash_t hashi(const CharType *str, size_t size) noexcept
        {
            const hash_t _FNV_offset_basis = __hash_basis;
            const hash_t _FNV_prime = __hash_prime;
            hash_t _Val = _FNV_offset_basis;
            for (hash_t _Next = 0; _Next < size; ++_Next)
            {
                const hash_t ch = str[_Next];
                if (ch == '\0')
                    continue;
                const hash_t hash_v = ch >= 'a' && ch <= 'z' ? ch + 'A' - 'a' : ch;
                _Val ^= (hash_t)hash_v;
                _Val *= _FNV_prime;
            }
            return _Val;
        }
    }
}

#pragma warning(pop)
