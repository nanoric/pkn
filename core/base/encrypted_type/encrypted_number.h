#pragma once

#include <stdint.h>
#include "../compile_time/random.h"
#include <random>

#pragma warning(push)
#pragma warning(disable:4307)
constexpr uint64_t __global_xor_key_for_encrypted_number = compile_time::random_from_seed(compile_time::hash(U"asldfkjasdjflzsnfsmfd/;zcvpoqwur0129u430l;asknmfd"));
#pragma warning(pop)

template <class T>
class const_encrypted_number
{
public:
    using internal_t = uint64_t;
private:
    internal_t _xor1;
    internal_t _xor2;
    internal_t _val;
    template <class R>
    friend class encrypted_number;
public:
    inline constexpr const_encrypted_number(const const_encrypted_number<T> &rhs) = default;
    inline constexpr const_encrypted_number(const T val, const compile_time::random_t random_offset)
        :
        _xor1(compile_time::random_from_seed(val^random_offset)),
        _xor2(compile_time::random_from_seed(compile_time::random_from_seed(val^random_offset))),
        _val(encrypt(val))
    {}
public:
    inline constexpr T value() const { return (T)(decrypt(_val)); }
private:
    inline constexpr internal_t internal_value() const { return _val; }
public:
    inline constexpr operator T () const { return value(); }
public:
    template <class R>
    inline constexpr internal_t encrypt(const R val) const noexcept
    {
        return val ^ _xor1 ^ _xor2;
    }
    template <class R>
    inline constexpr internal_t decrypt(const R val) const noexcept
    {
        return val ^ _xor1 ^ _xor2;
    }
};


template <class T>
class encrypted_number
{
public:
    using internal_t = uint64_t;
private:
    internal_t _xor1;
    internal_t _xor2;
    internal_t _val;
public:
    constexpr inline encrypted_number(const const_encrypted_number<T> &rhs) : _xor1(rhs._xor1), _xor2(rhs._xor2), _val(rhs._val) {}
    constexpr inline encrypted_number(const encrypted_number &rhs) : _xor1(rhs._xor1), _xor2(rhs._xor2), _val(rhs._val) {}
    constexpr inline encrypted_number() : _xor1(0), _xor2(0), _val(encrypt(0)) {}
    inline encrypted_number(const T val) : _xor1(gen_xor()), _xor2(gen_xor()), _val(encrypt(val)) {}
public:
    inline T value() const noexcept { return (T)(decrypt(_val)); }
    inline internal_t internal_value() const noexcept { return _val; }
public:
    inline operator T () const { return value(); }
private:
    template <class R>
    constexpr inline internal_t encrypt(const R val) const noexcept
    {
        return val ^ _xor1 ^ _xor2;
    }
    template <class R>
    constexpr inline internal_t decrypt(const R val) const noexcept
    {
        return val ^ _xor1 ^ _xor2;
    }
    static inline compile_time::random_t gen_xor() noexcept
    {
        static std::random_device rd;
        //static std::mersenne_twister_engine<unsigned long long, 64, 312, 156, 31,
        //    0xb5026f5aa96619e9ULL, 29,
        //    0x5555555555555555ULL, 17,
        //    0x71d67fffeda60000ULL, 37,
        //    0xfff7eee000000000ULL, 43,
        //    6364136223846793005ULL> gen(rd());
        //static std::linear_congruential_engine<unsigned long long,
        //    48271,
        //    0,
        //    0x7FFFFFFFFFFFFFFF> gen(rd());
        //static std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);
        //return dis(gen) ^ __global_xor_key_for_encrypted_number; // one more xor to prevent plain text be shown by "disable random" hack
        static uint64_t n = __global_xor_key_for_encrypted_number;
        constexpr uint64_t _1111prime = 281474976710717ULL;
        constexpr uint64_t _0101prime = 3074457345618258599ULL;
        n = (n << 1) ^ _0101prime;
        return n;
    }
};

namespace std
{
    template <typename T>
    struct hash<encrypted_number<T>>
    {
        inline constexpr uint64_t operator ()(encrypted_number<T> enc) const { return enc.value(); }
    };
}