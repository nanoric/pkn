#pragma once

#include <Windows.h>
#include <assert.h>

#include "../marcos/debug_print.h"

namespace pkn
{

inline bool double_working_set_size()
{
    SIZE_T mi, ma;
    if (!GetProcessWorkingSetSize(GetCurrentProcess(),
                                  &mi,
                                  &ma))
    {
        DebugPrint("Failed to GetProcessWorkingSetSize\n");
        return false;
    }
    mi *= 2;
    ma *= 2;
    if (!SetProcessWorkingSetSize(GetCurrentProcess(),
                                  mi,
                                  ma))
    {
        DebugPrint("Failed to SetProcessWorkingSetSize\n");
        return false;
    }
    return true;
}

inline void *allocate_nonpaged_memory(size_t size)
{
    auto ptr = VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    while (!VirtualLock(ptr, size))
    {
        DebugPrint("Failed to VirtualLock memory, trying to increase working set size\n");
        if (!double_working_set_size())
            return nullptr;
    }
    return ptr;
}

inline void free_nonpaged_memory(void *ptr, size_t size)
{
    assert(VirtualUnlock(ptr, size));
    assert(VirtualFree(ptr, 0, MEM_RELEASE));
}

template <class T>
class NonpagedMemory
{
    using FullType = T;
public:
    NonpagedMemory()
        : ptr(reinterpret_cast<T*>(allocate_nonpaged_memory(sizeof(T))))
    {}
    ~NonpagedMemory()
    {
        free_nonpaged_memory(ptr, sizeof(T));
    }
public:
    T *get() const noexcept { return ptr; }
    size_t size() const noexcept { return sizeof(T); }
    operator T*() const noexcept
    {
        return get();
    }
private:
    T *ptr;
};

template <class T>
class NonpagedMemory<T *>
{
    using FullType = T;
public:
    NonpagedMemory(size_t n)
        :_size(n * sizeof(T)), ptr(reinterpret_cast<T*>(allocate_nonpaged_memory(_size)))
    {}
    ~NonpagedMemory()
    {
        free_nonpaged_memory(ptr, _size);
    }
public:
    T *get() const noexcept { return ptr; }
    size_t size() const noexcept { return _size; }
    operator T*() const noexcept
    {
        return get();
    }
private:
    size_t _size;
    T *ptr;
};

template <class T, size_t n>
class NonpagedMemory<T[n]>
{
    using FullType = T[n];
public:
    NonpagedMemory()
        : ptr(reinterpret_cast<T*>(allocate_nonpaged_memory(sizeof(FullType))))
    {}
    ~NonpagedMemory()
    {
        free_nonpaged_memory(ptr, sizeof(FullType));
    }
public:
    T *get() const noexcept { return ptr; }
    constexpr size_t size() const noexcept { return sizeof(FullType); }
    operator T*() const noexcept
    {
        return get();
    }
private:
    T *ptr;
};

}
