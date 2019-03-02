#pragma once
#include <memory>
//#include "pknstl/unique_ptr"

extern "C"
{
    NTSYSAPI
    NTSTATUS
    NTAPI
    ZwQuerySystemInformation(
        IN UINT64 SystemInformationClass,
        OUT PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        OUT PULONG ReturnLength OPTIONAL
    );
}

NTSTATUS query_system_information(UINT64 informaiton_class, void *buffer, UINT64 buffer_size, ULONG * ret_size);
