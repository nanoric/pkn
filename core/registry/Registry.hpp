#pragma once

#include <pkn/core/base/types.h>
#include <pkn/core/marcos/debug_print.h>

#include "RegistryStructures.h"

#include <optional>
#include <memory>
#include <functional>

class Registry
{
public:
    using ZwCloseType = NTSTATUS(NTAPI*)(HANDLE);
public:
    Registry(const estr_t &registry_path, ZwCloseType zwClose)
        : _path(registry_path), zwClose(zwClose)
    {}
    virtual ~Registry()
    {
        close();
    }
public:
    estr_t path() const
    {
        return _path;
    }
public:
    bool open()
    {
        if (auto res = _open(this->path()))
        {
            this->_handle = *res;
            return true;
        }
        return false;
    }


    bool create()
    {
        UNICODE_STRING upath;
        OBJECT_ATTRIBUTES oa;
        uint32_t attr = OBJ_OPENIF | OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
        InitializeObjectAttributes(&oa, &upath, attr, nullptr, nullptr);
        auto path_ws = this->_path.to_wstring();
        RtlInitUnicodeString(oa.ObjectName, path_ws.c_str());

        auto status = this->zwCreateKey(&_handle,
                                        KEY_ALL_ACCESS,
                                        &oa,
                                        0,
                                        nullptr,
                                        0,
                                        nullptr);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("create registry %ls status: %x \n", path_ws.c_str(), status);
            return false;
        }
        return true;

    }

    void close()
    {
        if (is_opened())
        {
            this->zwClose(_handle);
            _handle = (HANDLE)-1;
        }
    }
    std::optional<std::vector<estr_t>> children()
    {
        std::vector<estr_t> cs;
        NTSTATUS status;
        for (int i = 0;; i++)
        {
            ULONG buffer_size = 0;
            // 1st enumerate : get size
            status = this->zwEnumerateKey(this->_handle,
                                          i,
                                          KeyBasicInformation,
                                          nullptr,
                                          0,
                                          &buffer_size);
            // break for no entry
            if (status == STATUS_NO_MORE_ENTRIES)
                break;
            if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL)
            {
                DebugPrint("1st ZwEnumerateKey:%ls failed with status : %x\n", path().to_wstring().c_str(), status);
                return std::nullopt;
            }

            // 2nd enumerate : get information: name
            buffer_size += 128; // since this is not a transation key handle, reverse some more space.
            std::unique_ptr<char> buffer(new char[buffer_size]);
            KEY_BASIC_INFORMATION *pkbi = (KEY_BASIC_INFORMATION *)buffer.get();
            status = this->zwEnumerateKey(this->_handle,
                                          i,
                                          KeyBasicInformation,
                                          pkbi,
                                          buffer_size,
                                          &buffer_size);
            if (status == STATUS_NO_MORE_ENTRIES)
                break;
            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
            {
                DebugPrint("2nd ZwEnumerateKey:%ls failed with status : %x\n", path().to_wstring().c_str(), status);
                return std::nullopt;
            }
            if (!NT_SUCCESS(status))
            {
                DebugPrint("2nd ZwEnumerateKey:%ls failed with status : %x\n", path().to_wstring().c_str(), status);
                return std::nullopt;
            }
            cs.push_back(estr_t(pkbi->Name, pkbi->NameLength / sizeof(*pkbi->Name)));
        }
        return cs;
    }

    bool remove()
    {
        if (!is_opened())
        {
            if (!open())
                return true;
        }

        NTSTATUS status;
        auto prefix = this->path();
        prefix.push_back(L'\\');
        if (auto cs = children(); cs && cs->size())
        {
            for (const auto &c : *cs)
            {
                if (auto res = this->_open(prefix + c))
                {
                    status = this->zwDeleteKey(*res);
                    if (!NT_SUCCESS(status))
                    {
                        DebugPrint("warning : delete registry subkey %ls failed with status: %x \n", c.to_wstring().c_str(), status);
                    }
                }
            }
        }

        status = this->zwDeleteKey(_handle);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("delete registry %ls failed with status: %x \n", path().to_wstring().c_str(), status);
            return false;
        }
        close();
        return true;
    }
    inline bool is_opened()const { return _handle != nullptr && _handle != (void *)-1; }
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
        return set<std::wstring>(key_name, value.to_wstring());
    }

    template <>
    bool set<std::wstring>(const estr_t &key_name, const std::wstring &value)
    {
        if (!is_opened())
            if (!open())
                return false;

        auto wkey = key_name.to_wstring();
        UNICODE_STRING s;
        RtlInitUnicodeString(&s, wkey.c_str());
        auto status = this->zwSetValueKey(_handle, &s, 0, REG_SZ, (void *)value.c_str(), (ULONG)(value.size() + 1) * sizeof(estr_t::basic_t));
        if (!NT_SUCCESS(status))
        {
            DebugPrint("set value (%ls) for registry key %ls in %ls status: %x \n",
                       value.c_str(),
                       wkey.c_str(),
                       path().to_wstring().c_str(),
                       status
            );
            return false;
        }
        return true;
    }
    template <>
    bool set<int>(const estr_t &key_name, const int &value)
    {
        uint32_t val = value;
        return set<uint32_t>(key_name, val);
    }

    template <>
    bool set<uint32_t>(const estr_t &key_name, const uint32_t &value)
    {
        if (!is_opened())
            if (!open())
                return false;

        auto wkey = key_name.to_wstring();
        UNICODE_STRING s;
        RtlInitUnicodeString(&s, wkey.c_str());
        auto status = this->zwSetValueKey(_handle, &s, 0, REG_DWORD, (void *)&value, 4);

        if (!NT_SUCCESS(status))
        {
            DebugPrint("set value (%u) for registry key %ls status: %x \n", value, key_name.to_wstring().c_str(), status);
            return false;
        }
        return true;
    }

    template <class T>
    std::optional<T> get(const estr_t &key_name)
    {
        static_assert(false, "This type is currently not supported, please implement it yourself");
    }
    template <>
    std::optional<estr_t> get<estr_t>(const estr_t &key_name)
    {
        // ensure opened
        if (!is_opened())
            if (!open())
                return std::nullopt;

        // 1st query: get length
        auto wkey = key_name.to_wstring();
        UNICODE_STRING s;
        RtlInitUnicodeString(&s, wkey.c_str());
        ULONG size = 0;
        auto status = this->zwQueryValueKey(_handle, &s, KeyValuePartialInformation, nullptr, 0, &size);
        if (status != STATUS_BUFFER_TOO_SMALL && status != STATUS_BUFFER_OVERFLOW)
        {
            DebugPrint("1st zwQueryValueKey:%ls failed with status : %x\n", path().to_wstring().c_str(), status);
            return std::nullopt;
        }

        // 2nd query: get value
        std::unique_ptr<KEY_VALUE_PARTIAL_INFORMATION> buf(reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION *>(new char[size]));
        status = this->zwQueryValueKey(_handle, &s, KeyValuePartialInformation, buf.get(), size, &size);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("2nd zwQueryValueKey:%ls failed with status : %x\n", path().to_wstring().c_str(), status);
            return std::nullopt;
        }

        // success
        return estr_t((const wchar_t *)buf->Data, (size_t)buf->DataLength / sizeof(wchar_t));
    }
protected:
    std::optional<HANDLE> _open(const estr_t &reg_path)
    {
        HANDLE h;
        UNICODE_STRING upath;
        OBJECT_ATTRIBUTES oa;
        uint32_t attr = OBJ_OPENIF | OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
        InitializeObjectAttributes(&oa, &upath, attr, nullptr, nullptr);
        auto path_ws = reg_path.to_wstring();
        RtlInitUnicodeString(oa.ObjectName, path_ws.c_str());

        auto status = this->zwOpenKey(&h,
                                      KEY_ALL_ACCESS,
                                      &oa);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("open registry %ls status: %x \n", path_ws.c_str(), status);
            return std::nullopt;
        }
        return h;
    }
protected:
    virtual NTSTATUS zwOpenKey(
        PHANDLE             KeyHandle,
        uint32_t            DesiredAccess,
        POBJECT_ATTRIBUTES  ObjectAttributes
    ) = 0;
    virtual NTSTATUS zwCreateKey(
        PHANDLE             KeyHandle,
        ACCESS_MASK         DesiredAccess,
        POBJECT_ATTRIBUTES  ObjectAttributes,
        ULONG               TitleIndex,
        PUNICODE_STRING     Class,
        ULONG               CreateOptions,
        PULONG              Disposition
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
    virtual NTSTATUS zwDeleteKey(
        HANDLE          KeyHandle
    ) = 0;
    virtual NTSTATUS zwEnumerateKey(
        HANDLE                KeyHandle,
        ULONG                 Index,
        KEY_INFORMATION_CLASS KeyInformationClass,
        PVOID                 KeyInformation,
        ULONG                 Length,
        PULONG                ResultLength
    ) = 0;
private:
    HANDLE _handle = (void *)-1;
    estr_t _path;
    ZwCloseType zwClose; // since virtual method can't be call at destructor, use function pointer instead
    //std::wstring path_ws;
};
