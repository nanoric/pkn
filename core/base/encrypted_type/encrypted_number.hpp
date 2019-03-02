#pragma once

#include <stdint.h>
#include <xhash>
#include "../compile_time/random.hpp"

#pragma warning(push)
#pragma warning(disable:4307)
constexpr uint64_t __global_xor_key_for_encrypted_number = compile_time::random_from_seed(compile_time::hash(U"asldfkjasdjflzsnfsmfd/;zcvpoqwur0129u430l;asknmfd"));
#pragma warning(pop)

template <int pad_size=sizeof(size_t)>
struct alignas(1) pad_helper
{
    char pad[pad_size];
};

template <>
struct alignas(1) pad_helper<0>
{
};

#pragma pack(push, 1)
template <class T>
class alignas(1) const_encrypted_number
{
public:
    using internal_t = T;
private:
    struct alignas(1) internal_layout
    {
    public:
        internal_t _xor1;
        internal_t _xor2;
        internal_t _val;
    };
private:
    internal_t _xor1;
    internal_t _xor2;
    internal_t _val;
    pad_helper<(sizeof(internal_layout) + sizeof(size_t) - 1) / sizeof(size_t) * sizeof(size_t) - sizeof(internal_layout)> pad{};

    template <class R>
    friend class encrypted_number;
public:
    inline constexpr const_encrypted_number(const const_encrypted_number<internal_t> &rhs) = default;
    inline constexpr const_encrypted_number(
        const internal_t val,
        const compile_time::random_t random_offset
    ) :
    _xor1((internal_t)compile_time::random_from_seed(val^random_offset)),
        _xor2((internal_t)compile_time::random_from_seed(compile_time::random_from_seed(val^random_offset))),
        _val(encrypt(val))
    {}
public:
    inline constexpr internal_t value() const {
        return (internal_t)(decrypt(_val));
    }
private:
    inline constexpr internal_t internal_value() const {
        return _val;
    }
public:
    inline constexpr operator internal_t () const {
        return value();
    }
public:
    inline constexpr internal_t encrypt(const internal_t val) const noexcept
    {
        return val ^ _xor1 ^ _xor2;
    }
    inline constexpr internal_t decrypt(const internal_t val) const noexcept
    {
        return val ^ _xor1 ^ _xor2;
    }
};

template <class T>
inline compile_time::random_t gen_xor() noexcept
{
    static_assert(false, "for faster string compulation, you should define your own gen_xor function for this type");
}

template <>
inline compile_time::random_t gen_xor<uint64_t>() noexcept
{
    static uint64_t n = __global_xor_key_for_encrypted_number;
    constexpr uint64_t _1111prime = 281474976710597ull;
    constexpr uint64_t _0101prime = 3074457345618258599ull;
    n = n * _0101prime + 1;
    return n;
}

template <>
inline compile_time::random_t gen_xor<char32_t>() noexcept
{
    static char32_t n = __global_xor_key_for_encrypted_number;
    constexpr char32_t _1111prime = 268435399ul;
    constexpr char32_t _0101prime = 1431655777ul;
    n = n * _0101prime + 1;
    return n;
}

template <>
inline compile_time::random_t gen_xor<wchar_t>() noexcept
{
    static wchar_t n = (wchar_t)(0xFFFF&__global_xor_key_for_encrypted_number);
    constexpr wchar_t _1111prime = 4093;
    constexpr wchar_t _0101prime = 21841;
    n = n * _0101prime + 1;
    return n;
}

template <>
inline compile_time::random_t gen_xor<char>() noexcept
{
    static unsigned char n = (unsigned char)(0xFF&__global_xor_key_for_encrypted_number);
    constexpr wchar_t _0101prime8 = 83;
    return n+=83;
}

template <>
inline compile_time::random_t gen_xor<uint32_t>() noexcept
{
    static uint32_t n = __global_xor_key_for_encrypted_number;
    constexpr uint32_t _1111prime = 268435399ul;
    constexpr uint32_t _0101prime = 1431655777ul;
    n = n * _0101prime + 1;
    return n;
}

template <>
inline compile_time::random_t gen_xor<uint16_t>() noexcept
{
    static uint16_t n = (wchar_t)(0xFFFF&__global_xor_key_for_encrypted_number);
    constexpr uint16_t _1111prime = 4093;
    constexpr uint16_t _0101prime = 21841;
    n = n * _0101prime + 1;
    return n;
}

template <>
inline compile_time::random_t gen_xor<uint8_t>() noexcept
{
    static uint8_t n = (unsigned char)(0xFF&__global_xor_key_for_encrypted_number);
    constexpr uint8_t _0101prime8 = 11;
    return n+=83;
}


template <class T>
class alignas(1) encrypted_number
{
public:
    using internal_t = T;
private:
    struct alignas(1) internal_layout
    {
        internal_t _xor1;
        internal_t _xor2;
        internal_t _val;
    };
private:
    internal_t _xor1;
    internal_t _xor2;
    internal_t _val;
    pad_helper<(sizeof(internal_layout) + sizeof(size_t) - 1) / sizeof(size_t) * sizeof(size_t) - sizeof(internal_layout)> pad;
public:
    inline encrypted_number(const internal_t val) :
        _xor1((internal_t)gen_xor<internal_t>()),
        _xor2((internal_t)gen_xor<internal_t>()),
        _val((internal_t)encrypt(val)) {}

    constexpr inline encrypted_number(const encrypted_number &rhs) = default;
    constexpr inline encrypted_number(const const_encrypted_number<internal_t> &rhs) : _xor1(rhs._xor1), _xor2(rhs._xor2), _val(rhs._val) {}
    constexpr inline encrypted_number() : _xor1(0), _xor2(0), _val((internal_t)encrypt(internal_t(0))) {}
public:
    inline internal_t value() const noexcept {
        return (internal_t)(decrypt(_val));
    }
    inline internal_t internal_value() const noexcept {
        return _val;
    }
public:
    inline operator internal_t () const {
        return value();
    }
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
};

#pragma pack(pop)

namespace std
{
template <typename internal_t>
struct hash<encrypted_number<internal_t>>
{
    inline constexpr uint64_t operator ()(encrypted_number<internal_t> enc) const {
        return enc.value();
    }
};
}