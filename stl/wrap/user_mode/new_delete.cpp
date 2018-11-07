#pragma once
#include <stdint.h>
#include <cstddef>
#include <memory>

void* _cdecl operator new(std::size_t size)
{
    size = size == 0 ? 1 : size;
    return malloc(size);
}

void* _cdecl operator new[](std::size_t size)
{
    size = size == 0 ? 1 : size;
    return malloc(size);
}

void _cdecl operator delete(void* ptr)
{
    return free(ptr);
}

void _cdecl operator delete[](void* ptr)
{
    return free(ptr);
}

void* operator new[](size_t size, const char*, int, unsigned, const char*, int)
{
    size = size == 0 ? 1 : size;
    return malloc(size);
}
void* operator new[](size_t size, size_t, size_t, const char*, int, unsigned, const char*, int)
{
    size = size == 0 ? 1 : size;
    return malloc(size);
}

void _cdecl operator delete(void* ptr, std::size_t)
{
    return free(ptr);
}

void _cdecl operator delete[](void* ptr, std::size_t)
{
    return free(ptr);
}

