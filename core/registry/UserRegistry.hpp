#pragma once

#include "Registry.hpp"


class UserRegistry : public Registry
{
public:
    using Registry::Registry;
protected:
    // Inherited via Registry

    virtual NTSTATUS UserRegistry::ZwOpenKey(PHANDLE KeyHandle,
                                     uint32_t DesiredAccess,
                                     POBJECT_ATTRIBUTES ObjectAttributes
    ) override
    {
        using fZwOpenKey = NTSTATUS(*)(PHANDLE KeyHandle,
                                       uint32_t DesiredAccess,
                                       POBJECT_ATTRIBUTES ObjectAttributes
                                       );
        static fZwOpenKey ZwOpenKey = (fZwOpenKey)GetProcAddress(LoadLibraryA("NtDll"), "ZwOpenKey");
        return ZwOpenKey(KeyHandle,
                         DesiredAccess,
                         ObjectAttributes);
    }

    virtual NTSTATUS UserRegistry::ZwQueryValueKey(HANDLE KeyHandle,
                                           PUNICODE_STRING ValueName,
                                           KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                                           PVOID KeyValueInformation,
                                           ULONG Length,
                                           PULONG ResultLength
    )
    {
        using fZwQueryValueKey = NTSTATUS(*)(HANDLE KeyHandle,
                                             PUNICODE_STRING ValueName,
                                             KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                                             PVOID KeyValueInformation,
                                             ULONG Length,
                                             PULONG ResultLength);
        static fZwQueryValueKey ZwQueryValueKey = (fZwQueryValueKey)GetProcAddress(LoadLibraryA("NtDll"), "ZwQueryValueKey");
        return ZwQueryValueKey(KeyHandle,
                               ValueName,
                               KeyValueInformationClass,
                               KeyValueInformation,
                               Length,
                               ResultLength);
    }

    virtual NTSTATUS Registry::ZwSetValueKey(HANDLE KeyHandle,
                                     PUNICODE_STRING ValueName,
                                     ULONG TitleIndex,
                                     ULONG Type,
                                     PVOID Data,
                                     ULONG DataSize
    ) override
    {
        using fZwSetValueKey = NTSTATUS(*)(HANDLE KeyHandle,
                                           PUNICODE_STRING ValueName,
                                           ULONG TitleIndex,
                                           ULONG Type,
                                           PVOID Data,
                                           ULONG DataSize);
        static fZwSetValueKey ZwSetValueKey = (fZwSetValueKey)GetProcAddress(LoadLibraryA("NtDll"), "ZwSetValueKey");
        return ZwSetValueKey(KeyHandle,
                               ValueName,
                               TitleIndex,
                               Type,
                               Data,
                               DataSize
        );
    }

    virtual NTSTATUS UserRegistry::ZwClose(HANDLE Handle) override
    {
        using fZwClose = NTSTATUS(*)(HANDLE Handle);
        static fZwClose ZwClose = (fZwClose)GetProcAddress(LoadLibraryA("NtDll"), "ZwClose");
        return ZwClose(Handle);
    }
};