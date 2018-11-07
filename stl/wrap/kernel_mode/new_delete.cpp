#pragma once

#include <ntifs.h>
#include <stdint.h>
#include <cstddef>

#define PTR_SIZE sizeof(void *)

void * operator new(std::size_t size)
{
    size = size == 0 ? 1 : size;
    //size_t adjustedAlignment = PTR_SIZE;

    //void* p = ExAllocatePoolWithTag(PagedPool, size + adjustedAlignment + PTR_SIZE, 'KANP');
    //void* pPlusPointerSize = (void*)((void **)p + 1);
    //void* pAligned = (void*)(((uint64_t)pPlusPointerSize + adjustedAlignment - 1) & ~(adjustedAlignment - 1));

    //void** pStoredPtr = (void**)pAligned - 1;
    //*(pStoredPtr) = p;

    //return pAligned;
    return ExAllocatePoolWithTag(PagedPool, size, 'KAAN');
}

void * operator new[](std::size_t size)
{
    size = size == 0 ? 1 : size;

    //size_t adjustedAlignment = PTR_SIZE;

    //void* p = ExAllocatePoolWithTag(PagedPool, size + adjustedAlignment + PTR_SIZE, 'KAAR');
    //void* pPlusPointerSize = (void*)((void **)p + 1);
    //void* pAligned = (void*)(((uint64_t)pPlusPointerSize + adjustedAlignment - 1) & ~(adjustedAlignment - 1));

    //void** pStoredPtr = (void**)pAligned - 1;
    //*(pStoredPtr) = p;

    //return pAligned;
    return ExAllocatePoolWithTag(PagedPool, size, 'KAAR');
}

void operator delete(void* ptr)
{
    if (nullptr != ptr)
    {
        ExFreePool(ptr);
    }
}

void operator delete[](void* ptr)
{
    if (nullptr != ptr)
    {
        ExFreePool(ptr);
    }
}

void* operator new[](size_t size,
                     const char* pName,
                     int flags,
                     unsigned debugFlags,
                     const char* file,
                     int line)
{
    size = size == 0 ? 1 : size;

    //size_t adjustedAlignment = PTR_SIZE;

    //void* p = ExAllocatePoolWithTag(PagedPool, size + adjustedAlignment + PTR_SIZE, 'KMA2');
    //void* pPlusPointerSize = (void*)((void **)p + 1);
    //void* pAligned = (void*)(((uint64_t)pPlusPointerSize + adjustedAlignment - 1) & ~(adjustedAlignment - 1));

    //void** pStoredPtr = (void**)pAligned - 1;
    //*(pStoredPtr) = p;

    //return pAligned;

    return ExAllocatePoolWithTag(PagedPool, size, 'KMAN');
}
void* operator new[](size_t size,
                     size_t alignment,
                     size_t alignmentOffset,
                     const char* pName,
                     int flags,
                     unsigned debugFlags,
                     const char* file,
                     int line)
{
    size = size == 0 ? 1 : size;

    //size_t adjustedAlignment = (alignment > PTR_SIZE) ? alignment : PTR_SIZE;

    //void* p = ExAllocatePoolWithTag(PagedPool, size + adjustedAlignment + PTR_SIZE, 'KMA2');
    //void* pPlusPointerSize = (void*)((void **)p + 1);
    //void* pAligned = (void*)(((uint64_t)pPlusPointerSize + adjustedAlignment - 1) & ~(adjustedAlignment - 1));

    //void** pStoredPtr = (void**)pAligned - 1;
    //*(pStoredPtr) = p;

    //return pAligned;

    return ExAllocatePoolWithTag(PagedPool, size, 'KMAA');
}

void operator delete(void* ptr, std::size_t)
{
    if (nullptr != ptr)
    {
        //void* pOriginalAllocation = *((void**)ptr - 1);
        //ExFreePool(pOriginalAllocation);
        ExFreePool(ptr);
    }
}

void operator delete[](void* ptr, std::size_t)
{
    //__debugbreak();
    if (nullptr != ptr)
    {
        //void* pOriginalAllocation = *((void**)ptr - 1);
        //ExFreePool(pOriginalAllocation);
        ExFreePool(ptr);
    }
}