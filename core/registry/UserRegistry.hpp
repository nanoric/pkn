#pragma once

#include "Registry.hpp"


class UserRegistry : public Registry
{
public:
    UserRegistry(const estr_t &registry_path)
        : Registry(registry_path, &UserRegistry::zwClose)
    {}
    virtual ~UserRegistry() override = default;
protected:
    // Inherited via Registry

    virtual NTSTATUS zwOpenKey(PHANDLE KeyHandle,
                                     uint32_t DesiredAccess,
                                     POBJECT_ATTRIBUTES ObjectAttributes
    ) override
    {
        using fZwOpenKey = NTSTATUS(NTAPI*)(PHANDLE KeyHandle,
                                       uint32_t DesiredAccess,
                                       POBJECT_ATTRIBUTES ObjectAttributes
                                       );
        static fZwOpenKey ZwOpenKey = (fZwOpenKey)GetProcAddress(LoadLibraryA("NtDll"), "ZwOpenKey");
        return ZwOpenKey(KeyHandle,
                         DesiredAccess,
                         ObjectAttributes);
    }

    virtual NTSTATUS zwQueryValueKey(HANDLE KeyHandle,
                                           PUNICODE_STRING ValueName,
                                           KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                                           PVOID KeyValueInformation,
                                           ULONG Length,
                                           PULONG ResultLength
    )
    {
        using fZwQueryValueKey = NTSTATUS(NTAPI*)(HANDLE KeyHandle,
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

    virtual NTSTATUS zwSetValueKey(HANDLE KeyHandle,
                                     PUNICODE_STRING ValueName,
                                     ULONG TitleIndex,
                                     ULONG Type,
                                     PVOID Data,
                                     ULONG DataSize
    ) override
    {
        using fZwSetValueKey = NTSTATUS(NTAPI*)(HANDLE KeyHandle,
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

    static NTSTATUS zwClose(HANDLE Handle)
    {
        using fZwClose = NTSTATUS(NTAPI*)(HANDLE Handle);
        static fZwClose ZwClose = (fZwClose)GetProcAddress(LoadLibraryA("NtDll"), "ZwClose");
        return ZwClose(Handle);
    }
};