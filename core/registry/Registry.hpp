#pragma once

#ifndef _KERNEL_MODE
#include <Windows.h>
#include <winternl.h>
typedef enum _KEY_VALUE_INFORMATION_CLASS
{
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    KeyValueLayerInformation,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;
typedef struct _KEY_VALUE_PARTIAL_INFORMATION
{
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    _Field_size_bytes_(DataLength) UCHAR Data[1]; // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

#ifndef STATUS_BUFFER_TOO_SMALL          
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#endif
#ifndef STATUS_BUFFER_OVERFLOW           
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#endif

#endif

#include <pkn/core/base/types.h>
#include <pkn/stl/optional>
#include <pkn/stl/unique_ptr>

class Registry
{
public:
    Registry(const estr_t &registry_path) : path(registry_path)
    {
    }
    ~Registry()
    {
        if (is_opened())
            close();
    }
public:
    bool open()
    {
        UNICODE_STRING upath;
        oa.Attributes = OBJ_KERNEL_HANDLE;
        oa.Length = sizeof(oa);
        oa.ObjectName = &upath;
        oa.RootDirectory = nullptr;
        oa.SecurityDescriptor = nullptr;
        oa.SecurityQualityOfService = nullptr;
        RtlInitUnicodeString(oa.ObjectName, this->path.to_wstring().c_str());

        auto status = this->ZwOpenKey(&handle, KEY_READ, &oa);
        return NT_SUCCESS(status);
    }
    void close()
    {
        this->ZwClose(handle);
        handle = (HANDLE)-1;
    }
    inline bool is_opened()const { return handle != nullptr && handle != (void *)-1; }
public:
    // todo: use template function set<type>
    template <class T>
    bool set(const estr_t &key_name, const T &value)
    {
        static_assert(false, "This type is currently not supported, please implement it yourself");
    }

    template <>
    bool set<estrv_t>(const estr_t &key_name, const estrv_t &value)
    {
        return set<estr_t>(key_name, value);
    }

    template <>
    bool set<estr_t>(const estr_t &key_name, const estr_t &value)
    {
        if (!is_opened())
            if (!open())
                return false;

        auto wkey = key_name.to_wstring();
        UNICODE_STRING s;
        RtlInitUnicodeString(&s, wkey.c_str());
        auto ws = value.to_wstring();
        auto status = this->ZwSetValueKey(handle, &s, 0, REG_SZ, (void *)ws.c_str(), (ULONG)(ws.size() + 1) * sizeof(estr_t::basic_t));
        if (NT_SUCCESS(status))
        {
            return true;
        }
        return false;
    }

    template <class T>
    stl::optional<T> get(const estr_t &key_name)
    {
        static_assert(false, "This type is currently not supported, please implement it yourself");
    }
    template <>
    stl::optional<estr_t> get<estr_t>(const estr_t &key_name)
    {
        if (!is_opened())
            if (!open())
                return stl::nullopt;
        auto wkey = key_name.to_wstring();
        UNICODE_STRING s;
        RtlInitUnicodeString(&s,wkey.c_str());
        ULONG size = 0;
        auto status = this->ZwQueryValueKey(handle, &s, KeyValuePartialInformation, nullptr, 0, &size);
        if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)
        {
            stl::unique_ptr<KEY_VALUE_PARTIAL_INFORMATION> buf(reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION *>(new char[size]));
            status = this->ZwQueryValueKey(handle, &s, KeyValuePartialInformation, buf.get(), size, &size);
            if (NT_SUCCESS(status))
            {
                return estr_t((const wchar_t *)buf->Data, (size_t)buf->DataLength / sizeof(wchar_t));
            }
        }
        return stl::nullopt;
    }
protected:
    virtual NTSTATUS ZwOpenKey(
        PHANDLE            KeyHandle,
        uint32_t        DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes
    ) = 0;
    virtual NTSTATUS ZwQueryValueKey(
        HANDLE                      KeyHandle,
        PUNICODE_STRING             ValueName,
        KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        PVOID                       KeyValueInformation,
        ULONG                       Length,
        PULONG                      ResultLength
    ) = 0;
    virtual NTSTATUS ZwSetValueKey(
        HANDLE          KeyHandle,
        PUNICODE_STRING ValueName,
        ULONG           TitleIndex,
        ULONG           Type,
        PVOID           Data,
        ULONG           DataSize
    ) = 0;
    virtual NTSTATUS ZwClose(
        HANDLE Handle
    ) = 0;
private:
    HANDLE handle = (void *)-1;
    estr_t path;
    OBJECT_ATTRIBUTES oa;
};
