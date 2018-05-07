#pragma once

#include <stdint.h>
#include "hash.h"
#pragma warning(push)
#pragma warning(disable : 4307)

namespace compile_time
{
    using random_t = uint64_t;

    inline constexpr uint32_t random32_from_seed(uint32_t seed)
    {
        constexpr uint32_t prime = 16777259ul;
        constexpr uint32_t offset = 2UL;
        constexpr uint32_t mod = 0xFFFFFFFF;
        return (uint32_t)(0xFFFFFFFF & ((0xFFFFFFFF & ((uint64_t)seed * prime)) + offset) % mod);
    }


    inline constexpr random_t random_from_seed(random_t seed)
    {
        return random32_from_seed(seed & 0xFFFFFFFF) | random32_from_seed((seed >> 32) & 0xFFFFFFFF);
    }


    /*
    * I don't know if each call to this may the same or differ event in the same project, dependents on compile start time in seconds
    * If you need a fully different random number, use linear_generator with random()
    * If you need just one random number, use a runtime generated variable to store it
    */
    inline constexpr random_t random_daily()
    {
        constexpr random_t seed = hash(__DATE__);
        return random_from_seed(seed);
    }

    /*
    * I don't know if each call to this may the same or differ even in the same project,
    * result dependents on compile start time in seconds
    * If you need a fully different random number, use linear_generator with random()
    * If you need just one random number, use a runtime generated variable to store it
    * in MSVC, a global constexpr value has only one value all over "a build"
    *   rebuild will make this value different.
    */
    inline constexpr random_t random()
    {
        constexpr random_t seed = hash(__DATE__) | hash(__TIME__);
        return random_from_seed(seed ^ __COUNTER__);
    }
}

#pragma warning(pop)
