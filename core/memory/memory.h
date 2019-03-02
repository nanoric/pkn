#pragma once

#include <random>

inline void fill_random(void *address, size_t size)
{
    static std::random_device d;
    static std::mt19937_64 mt(d()); // get random number between [0, 2^64-1] don't need a uniform_int_distribution
    int i = 0;
    char *p = (char *)address;
    if (size >= 8)
    {
        for (i, p; i < size - 7; i += 8)
        {
            *(uint64_t *)(p + i) = mt();
        }
    }
    for (i, p; i < size; i++)
    {
        *(uint8_t *)(p + i) = uint8_t(mt());
    }
}