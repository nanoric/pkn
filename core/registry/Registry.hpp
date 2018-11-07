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

#else
#endif
#include <pkn/core/marcos/debug_print.h>

#include <pkn/core/base/types.h>
#include <pkn/stl/optional>
#include <pkn/stl/unique_ptr>
#include <pkn/stl/functional>

class Registry
{
public:
    using ZwCloseType = NTSTATUS(NTAPI*)(HANDLE);
public:
    Registry(const estr_t &registry_path, ZwCloseType zwClose)
        : path(registry_path), zwClose(zwClose)
    {}
    virtual ~Registry()
    {
        close();
    }
public:
    bool open()
    {
        UNICODE_STRING upath;
        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(&oa, &upath, OBJ_KERNEL_HANDLE, NULL, NULL);
        auto path_ws = this->path.to_wstring();
        RtlInitUnicodeString(oa.ObjectName, path_ws.c_str());

        auto status = this->zwOpenKey(&handle, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY, &oa);
        DebugPrint("ZwOpenKey handle:(%llx)", handle);
        return NT_SUCCESS(status);
    }
    void close()
    {
        if (is_opened())
        {
            this->zwClose(handle);
            handle = (HANDLE)-1;
        }
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
        auto status = this->zwSetValueKey(handle, &s, 0, REG_SZ, (void *)ws.c_str(), (ULONG)(ws.size() + 1) * sizeof(estr_t::basic_t));
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
        RtlInitUnicodeString(&s, wkey.c_str());
        ULONG size = 0;
        auto status = this->zwQueryValueKey(handle, &s, KeyValuePartialInformation, nullptr, 0, &size);
        if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)
        {
            stl::unique_ptr<KEY_VALUE_PARTIAL_INFORMATION> buf(reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION *>(new char[size]));
            status = this->zwQueryValueKey(handle, &s, KeyValuePartialInformation, buf.get(), size, &size);
            if (NT_SUCCESS(status))
            {
                return estr_t((const wchar_t *)buf->Data, (size_t)buf->DataLength / sizeof(wchar_t));
            }
        }
        return stl::nullopt;
    }
protected:
    virtual NTSTATUS zwOpenKey(
        PHANDLE            KeyHandle,
        uint32_t        DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes
    ) = 0;
    virtual NTSTATUS zwQueryValueKey(
        HANDLE                      KeyHandle,
        PUNICODE_STRING             ValueName,
        KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        PVOID                       KeyValueInformation,
        ULONG                       Length,
        PULONG                      ResultLength
    ) = 0;
    virtual NTSTATUS zwSetValueKey(
        HANDLE          KeyHandle,
        PUNICODE_STRING ValueName,
        ULONG           TitleIndex,
        ULONG           Type,
        PVOID           Data,
        ULONG           DataSize
    ) = 0;
private:
    HANDLE handle = (void *)-1;
    estr_t path;
    ZwCloseType zwClose; // since virtual method can't be call at destructor, use function pointer instead
    //stl::wstring path_ws;
};
