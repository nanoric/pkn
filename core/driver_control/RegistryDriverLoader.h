#include "../base/types.h"

#include <filesystem>

#include "../registry/UserRegistry.hpp"
#include "../base/fs/fsutils.h"

/*
use load() to load driver.
driver will be auto unloaded when object is destructed.
*/
namespace pkn
{
class RegistryDriverLoader
{
public:
    RegistryDriverLoader(const estr_t &registry_entry_name, const estr_t &driver_path)
        :_registry_entry_name(registry_entry_name), _driver_path(driver_path)
    {}
    ~RegistryDriverLoader()
    {
        stop();
        unload();
    }
public:
    bool create()
    {
        if (!created)
        {
            enable_load_driver_privilege();
            created = create_no_check();
        }
        return created;
    }
    bool load()
    {
        if (create())
        {
            if (!loaded)
            {
                loaded = start_no_check();
            }
        }
        return loaded;
    }
    bool stop()
    {
        if (loaded)
        {
            loaded = !stop_no_check();
        }
        return !loaded;
    }
    bool unload()
    {
        if (stop())
        {
            if (created)
            {
                created = !remove_no_check();
            }
        }
        return !loaded && !created;
    }
    const estr_t &driver_path() const noexcept
    {
        return _driver_path;
    }
    estr_t driver_filename()
    {
        return pkn::filename_for_path(_driver_path);
    }

    inline UserRegistry registry()
    {
        return UserRegistry(registry_path());
    }

    estr_t registry_path()
    {
        estr_t path = make_estr(R"(\Registry\Machine\SYSTEM\CurrentControlSet\Services\)");
        path += _registry_entry_name;
        return path;
    }

protected:
    bool create_no_check()
    {
        auto reg = this->registry();
        if (!reg.create())
            return false;

        auto full_path = std::filesystem::absolute(_driver_path.to<std::wstring>());
        estr_t kernel_path = estr_t(make_estr(R"(\??\)")) + estr_t(full_path.wstring());
        if (!reg.set(make_estr("ErrorControl"), 1))
            return false;
        if (!reg.set(make_estr("ImagePath"), kernel_path))
            return false;
        if (!reg.set(make_estr("Start"), 3))
            return false;
        if (!reg.set(make_estr("Type"), 1))
            return false;
        return true;
    }
    bool start_no_check()
    {
        using fZwLoadDriver = NTSTATUS(NTAPI*)(PUNICODE_STRING Handle);
        static fZwLoadDriver ZwLoadDriver = (fZwLoadDriver)
            GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                           make_const_encrypted_string("ZwLoadDriver").to_string().c_str());
        UNICODE_STRING us;
        auto ws = this->registry_path().to_wstring();
        RtlInitUnicodeString(&us, ws.c_str());
        auto status = ZwLoadDriver(&us);

#ifndef STATUS_IMAGE_ALREADY_LOADED
#define STATUS_IMAGE_ALREADY_LOADED 0xC000010E
#endif
        if (!NT_SUCCESS(status) && STATUS_IMAGE_ALREADY_LOADED != status)
        {
            DebugPrint("load driver status : %x\n", status);
            return false;
        }

        return true;
    }
    bool stop_no_check()
    {
        using fZwUnloadDriver = NTSTATUS(NTAPI*)(PUNICODE_STRING Handle);
        static fZwUnloadDriver ZwUnloadDriver = (fZwUnloadDriver)
            GetProcAddress(LoadLibraryW(make_estr("NtDll").to_wstring().c_str()),
                           make_const_encrypted_string("ZwUnloadDriver").to_string().c_str());
        UNICODE_STRING us;
        auto ws = this->registry_path().to_wstring();
        RtlInitUnicodeString(&us, ws.c_str());
        auto status = ZwUnloadDriver(&us);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("unload driver status : %x\n", status);
            return false;
        }
        return true;
    }
    bool remove_no_check()
    {
        enable_load_driver_privilege();
        return this->registry().remove();
    }
public:
    static bool enable_load_driver_privilege();

public:
    bool created = false;
    bool loaded = false;
    estr_t _driver_path;
    estr_t _registry_entry_name;
};

}
