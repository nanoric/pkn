#include "IAddressableProcess.h"
#include "..\base\fs\fsutils.h"
#include <algorithm>

namespace pkn
{

void IProcessRegions::init()
{
    _retrive_memory_regions();
}

void IProcessRegions::refresh_regions()
{
    _clear_regions();
    _retrive_memory_regions();
}

MemoryRegions IProcessRegions::file_regions(const estr_t &executable_name) const
{
    auto regions = memory_regions();
    MemoryRegions results;
    std::copy_if(regions.begin(), regions.end(), std::back_inserter(results), [&, this](const MemoryRegion &region)
                 {
                     if (auto res = mapped_file(region))
                     {
                         estr_t file = *res;
                         file = filename_for_path(file);
                         if (file == executable_name)
                             return true;
                     }
                     return false;
                 });
    return results;
}

MemoryRegions IProcessRegions::file_regionsi(const estr_t &executable_name) const
{
    auto ln = executable_name.to_lower();
    auto regions = memory_regions();
    MemoryRegions results;
    std::copy_if(regions.begin(), regions.end(), std::back_inserter(results), [&, this](const MemoryRegion &region)
                 {
                     if (auto res = mapped_file(region))
                     {
                         estr_t file = *res;
                         file = filename_for_path(file).to_lower();
                         if (file == ln)
                             return true;
                     }
                     return false;
                 });
    return results;
}
const MemoryRegions &IProcessRegions::memory_regions() const
{
    return _regions;
}

const MemoryRegions &IProcessRegions::readable_regions() const
{
    return _readable_regions;
}

const MemoryRegions &IProcessRegions::readwritable_regions() const
{
    return _readwriteable_regions;
}

const pkn::MemoryRegions & IProcessRegions::readexecutable_regions() const
{
    return _readexecutable_regions;
}

const pkn::MemoryRegions & IProcessRegions::readwritexecutable_regions() const
{
    return _readwritexecutable_regions;
}

std::optional<estr_t> IProcessRegions::mapped_file_for_address(const erptr_t &remote_address) const 
{
    if (auto res = region_for_address(remote_address))
    {
        return mapped_file_for_base(res->base);
    }
    return std::nullopt;
}

std::optional<estr_t> IProcessRegions::mapped_file_for_base(const erptr_t &remote_base_address) const
{
    auto i = _mapped_file.find(remote_base_address);
    if (i != _mapped_file.cend())
    {
        return i->second;
    }
    return std::nullopt;
}

std::optional<estr_t> IProcessRegions::mapped_file(const MemoryRegion &region) const
{
    return mapped_file_for_base(region.base);
}

std::optional<pkn::MemoryRegion> IProcessRegions::region_for_address(const erptr_t &remote_address) const
{
    auto it = std::upper_bound(_regions.cbegin(), _regions.cend(), remote_address);
    if (it != _regions.cbegin())
    {
        return *(--it);
    }
    return std::nullopt;
}

constexpr inline int msb(uint64_t val)
{
    int i = 0;
    while (val)
    {
        val >>= 1;
        ++i;
    }
    return i;
}

void IProcessRegions::_clear_regions()
{
    _regions.clear();
    _readable_regions.clear();
    _readwriteable_regions.clear();
    _readexecutable_regions.clear();
    _readwritexecutable_regions.clear();
    _mapped_file.clear();
}

void IProcessRegions::_retrive_memory_regions()
{
    this->_clear_regions();
    _regions = get_all_memory_regions();
    std::sort(_regions.begin(), _regions.end());
    for (const auto &region : _regions)
    {
        if (region.readable())
            _readable_regions.push_back(region);
        if (region.readable() && region.writable())
            _readwriteable_regions.push_back(region);
        if (region.readable() && region.executable())
            _readexecutable_regions.push_back(region);
        if (region.readable() && region.writable() && region.executable())
            _readwritexecutable_regions.push_back(region);

        if (region.type == MEM_IMAGE)
        {
            estr_t image_path;
            if (get_mapped_file(region.base, &image_path))
            {
                auto base_name = filename_for_path(image_path);
                _mapped_file[region.base] = base_name;
            }
        }
    }
}

void ProcessAddressTypeInfo::_retrive_memory_informations(IProcessBasic *_basic_process, IProcessRegions *_addressable_process)
{
    process_base = _basic_process->base();
    auto process_base_msb = msb(process_base);
    auto memory_type_mask_bit = 4;
    auto memory_all_mask = (1i64 << (process_base_msb)) - 1;
    auto memory_lower_mask = (1i64 << (process_base_msb - memory_type_mask_bit)) - 1;
    memory_type_mask = memory_all_mask - memory_lower_mask;
    process_executable_memory_type_mask = process_base & memory_type_mask;

    if (auto res = _addressable_process->mapped_file_for_base(process_base))
    {
        estr_t image_path = *res;
        auto main_file_name = filename_for_path(image_path);
        _main_regions = _addressable_process->file_regions(main_file_name);
    }
}

void ProcessAddressTypeInfo::init(IProcessBasic *_basic_process, IProcessRegions *_addressable_process)
{
    _retrive_memory_informations(_basic_process, _addressable_process);
}

MemoryRegions ProcessAddressTypeInfo::main_file_regions() const
{
    return _main_regions;
}

bool ProcessAddressTypeInfo::seems_heap_address(rptr_t address) const
{
    return address != rnullptr && address < process_base && (address & memory_type_mask) != process_executable_memory_type_mask;
}

bool ProcessAddressTypeInfo::seems_executable_address(rptr_t address) const
{
    return (address & memory_type_mask) == process_executable_memory_type_mask;
    //return address > 0x7f0000000000 && address < 0x7FFF00000000;
}

bool ProcessAddressTypeInfo::is_address_seems_near(rptr_t p1, rptr_t p2) const
{
    if (seems_executable_address(p1))
        return seems_executable_address(p2) && std::abs((int64_t)(p1 - p2)) < 0x80000000;
    else
        return !seems_executable_address(p2) && std::abs((int64_t)(p1 - p2)) < 0x80000000;
}

}
