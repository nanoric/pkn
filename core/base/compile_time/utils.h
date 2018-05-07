#pragma once


template <int ... idx>
struct indexes
{};

template <size_t nLeft, int ... idx>
struct indexes_builder
{
    using result = typename indexes_builder<nLeft - 1, nLeft - 1, idx ...>::result;
};
template <int ... idx>
struct indexes_builder<0, idx ...>
{
    using result = indexes<idx ...>;
};

template <class string_t>
struct char_type_of_string_literal
{
};

template <class char_t, size_t size>
struct char_type_of_string_literal<const char_t(&)[size]>
{
    using result = char_t;
};

template <class char_t, size_t size>
struct char_type_of_string_literal<char_t(&)[size]>
{
    using result = char_t;
};

