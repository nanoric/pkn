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
        static fZwOpenKey ZwOpenKey = (fZwOpenKey)GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                                                                 make_const_encrypted_string("ZwOpenKey").to_string().c_str());
        return ZwOpenKey(KeyHandle,
                         DesiredAccess,
                         ObjectAttributes);
    }

    virtual NTSTATUS zwCreateKey(PHANDLE KeyHandle,
                                 ACCESS_MASK DesiredAccess,
                                 POBJECT_ATTRIBUTES ObjectAttributes,
                                 ULONG TitleIndex,
                                 PUNICODE_STRING Class,
                                 ULONG CreateOptions,
                                 PULONG Disposition) override
    {
        using fZwCreateKey = NTSTATUS(NTAPI*)(PHANDLE KeyHandle,
                                              ACCESS_MASK DesiredAccess,
                                              POBJECT_ATTRIBUTES ObjectAttributes,
                                              ULONG TitleIndex,
                                              PUNICODE_STRING Class,
                                              ULONG CreateOptions,
                                              PULONG Disposition
                                              );
        static fZwCreateKey ZwCreateKey = (fZwCreateKey)GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                                                                       make_const_encrypted_string("ZwCreateKey").to_string().c_str());
        return ZwCreateKey(KeyHandle,
                           DesiredAccess,
                           ObjectAttributes,
                           TitleIndex,
                           Class,
                           CreateOptions,
                           Disposition);
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
        static fZwQueryValueKey ZwQueryValueKey = (fZwQueryValueKey)GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                                                                                   make_const_encrypted_string("ZwQueryValueKey").to_string().c_str());
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
        static fZwSetValueKey ZwSetValueKey = (fZwSetValueKey)GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                                                                             make_const_encrypted_string("ZwSetValueKey").to_string().c_str());
        return ZwSetValueKey(KeyHandle,
                             ValueName,
                             TitleIndex,
                             Type,
                             Data,
                             DataSize
        );
    }

    virtual NTSTATUS zwDeleteKey(HANDLE Handle
    ) override
    {
        using fZwDeleteKey = NTSTATUS(NTAPI*)(HANDLE Handle);
        static fZwDeleteKey ZwDeleteKey = (fZwDeleteKey)GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                                                                       make_const_encrypted_string("ZwDeleteKey").to_string().c_str());
        return ZwDeleteKey(Handle);
    }

    static NTSTATUS zwClose(HANDLE Handle)
    {
        using fZwClose = NTSTATUS(NTAPI*)(HANDLE Handle);
        static fZwClose ZwClose = (fZwClose)GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                                                           make_const_encrypted_string("ZwClose").to_string().c_str());
        return ZwClose(Handle);
    }
};