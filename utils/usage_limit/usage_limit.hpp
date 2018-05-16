#pragma once

#include "../../core/base/compile_time/random.h"


#define seconds_limit(BASE, LIMIT_SECONDS) \
{\
    using namespace std::chrono;\
    constexpr uint64_t base = BASE;\
    constexpr uint64_t rnd_base = base + compile_time::random_from_seed(compile_time::random() + __COUNTER__) % 1000;\
    constexpr uint64_t limit = rnd_base + LIMIT_SECONDS + compile_time::random_from_seed(compile_time::random() + __COUNTER__) % 1000;\
    if(duration_cast<seconds>(system_clock::now().time_since_epoch()).count() > limit)\
    {\
        unsigned char code[] = { 0x48, 0x31, 0xC0, 0x48, 0x89, 0xE7, 0x48, 0xC7, 0xC1, 0x64, 0x00, 0x00, 0x00, 0xF3, 0x48, 0xAB, 0x48, 0x31, 0xDB, 0x48, 0x31, 0xC9, 0x48, 0x31, 0xD2, 0x4D, 0x31, 0xC0, 0x4D, 0x31, 0xC9, 0x48, 0x31, 0xF6, 0x48, 0x31, 0xFF, 0x48, 0x31, 0xE4, 0x48, 0x31, 0xED, 0xFF, 0xE0 };\
        using Fnc = void(*)();\
        Fnc f = (Fnc)(void *)&code[0];\
        f();\
    }\
}
