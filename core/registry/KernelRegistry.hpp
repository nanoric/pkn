#pragma once

#include "Registry.hpp"

class KernelRegistry : public Registry
{
public:
    using Registry::Registry;
protected:
    // Inherited via Registry

    virtual NTSTATUS ZwOpenKey(PHANDLE KeyHandle,
                       uint32_t DesiredAccess,
                       POBJECT_ATTRIBUTES ObjectAttributes
    ) override
    {
        return ::ZwOpenKey(KeyHandle,
                           DesiredAccess,
                           ObjectAttributes);
    }

    virtual NTSTATUS ZwQueryValueKey(HANDLE KeyHandle,
                             PUNICODE_STRING ValueName,
                             KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                             PVOID KeyValueInformation,
                             ULONG Length,
                             PULONG ResultLength
    ) override
    {
        return ::ZwQueryValueKey(KeyHandle,
                                 ValueName,
                                 KeyValueInformationClass,
                                 KeyValueInformation,
                                 Length,
                                 ResultLength
        );
    }

    virtual NTSTATUS ZwSetValueKey(HANDLE KeyHandle,
                           PUNICODE_STRING ValueName,
                           ULONG TitleIndex,
                           ULONG Type,
                           PVOID Data,
                           ULONG DataSize
    ) override
    {
        return ::ZwSetValueKey(KeyHandle,
                               ValueName,
                               TitleIndex,
                               Type,
                               Data,
                               DataSize
        );
    }

    virtual NTSTATUS ZwClose(HANDLE Handle) override
    {
        return ::ZwClose(Handle);
    }
};