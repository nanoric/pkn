#include "../driver_control/DriverBase.h"
#include "../injector/injector.hpp"
#include "../base/fs/fsutils.h"

#include "UserProcess.h"
namespace pkn
{
UserProcessBase::UserProcessBase(pid_t pid)
    : _pid(pid)
{}

UserProcessBase::~UserProcessBase()
{
    close();
}

bool UserProcessBase::open(DWORD access_mask)
{
    _handle = OpenProcess(access_mask, false, (DWORD)pid());
    if (_handle != nullptr)
        return true;
    return false;
}

void UserProcessBase::close()
{
    if (_handle)
    {
        CloseHandle(_handle);
        _handle = nullptr;
    }
}

UserBasicProcess::UserBasicProcess(pid_t pid)
    : UserProcessBase(pid),
    // todo: base is incorrect, EnumProcessModules is necessary
    _base(0)
{
}

bool UserBasicProcess::init()
{
    HMODULE modules[1024];
    DWORD cbNeeded;
    if (!EnumProcessModules(handle(), modules, sizeof(modules), &cbNeeded))
        return false;

    wchar_t main_filename[1024];
    if (0 == GetProcessImageFileNameW(handle(), main_filename, sizeof(main_filename)))
        return false;

    std::wstring std_main_filename(main_filename);
    std_main_filename = pkn::filename_for_path(std_main_filename);

    for (int i = 0; i < cbNeeded / sizeof(HMODULE) ; i++)
    {
        auto hmodule = modules[i];
        wchar_t filename[1024];
        if (0 == GetModuleFileNameExW(handle(), hmodule, filename, sizeof(filename)))
            continue;
        if (wcsstr(filename, std_main_filename.c_str()) != 0)
        {
            _base = (rptr_t)hmodule;
            return true;
        }
    }
    return false;
}


erptr_t UserBasicProcess::base() const
{
    return this->_base;
}

bool UserBasicProcess::alive() const
{
    return WaitForSingleObject(handle(), 0) == STATUS_TIMEOUT;
}

pid_t UserBasicProcess::pid() const 
{
    return pid();
}

bool UserReadableProcess::read_unsafe(const erptr_t &address, size_t size, void *buffer) const
{
    size_t nread;
    return ReadProcessMemory(handle(), (void *)(rptr_t)address, buffer, size, &nread);
}

bool UserWritableProcess::write_unsafe(erptr_t address, size_t size, const void *buffer) const
{
    size_t nread;
    return WriteProcessMemory(handle(), (void *)(rptr_t)address, buffer, size, &nread);
}

MemoryRegions UserProcessRegions::get_all_memory_regions()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    auto address_low = (rptr_t)si.lpMinimumApplicationAddress;
    auto address_high = (rptr_t)si.lpMaximumApplicationAddress;
    auto page_size = si.dwAllocationGranularity;

    MemoryRegions regions;
    for (rptr_t p = address_low; p < address_high;)
    {
        MEMORY_BASIC_INFORMATION mbi;
        bool success = false;
        if (0 == VirtualQueryEx(handle(), (void *)(rptr_t)p, &mbi, sizeof(mbi)))
        {
            p += page_size;
            continue;;
        }

        size_t memSize = mbi.RegionSize;
        if (mbi.State == MEM_COMMIT)
        {
            MemoryRegion region;
            region.base = (rptr_t)mbi.BaseAddress;
            region.size = (size_t)mbi.RegionSize;
            region.protect = (uint32_t)mbi.Protect;
            region.allocation_base = (rptr_t)mbi.AllocationBase;
            region.type = mbi.Type;
            regions.emplace_back(region);
        }
        p = p + memSize;
    }
    return regions;
}

bool UserProcessRegions::get_mapped_file(erptr_t remote_address, estr_t *mapped_file) const
{
    wchar_t filename[1024];
    if (0 == GetMappedFileNameW(handle(), (void *)remote_address.value(), filename, sizeof(filename)))
        return false;
    *mapped_file = filename;
    return true;
}

erptr_t UserExtraProcess::get_peb_address() const
{
    // todo: 
    return 0;
}

erptr_t UserExtraProcess::get_teb_address(pid_t thread_id) const
{
    // todo: 
    return 0;
}

}
