#include "IAddressableProcess.h"
#include "..\base\fs\fsutils.h"

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
            estr_t file;
            if (mapped_file(region.base, &file))
            {
                file = file_base_name(file);
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
            estr_t file;
            if (mapped_file(region.base, &file))
            {
                file = file_base_name(file).to_lower();
                if (file == ln)
                    return true;
            }
            return false;
        });
        return results;
    }
    MemoryRegions IProcessRegions::memory_regions() const
    {
        return _regions;
    }

    MemoryRegions IProcessRegions::readable_regions() const
    {
        return _readable_regions;
    }

    MemoryRegions IProcessRegions::readwritable_regions() const
    {
        return _readwriteable_regions;
    }

    bool IProcessRegions::mapped_file(erptr_t remote_address, estr_t *image_path) const
    {
        auto i = _mapped_file.lower_bound(remote_address);
        if (i != _mapped_file.end() && i->first == remote_address)
        {
            *image_path = i->second;
            return true;
        }
        return false;
    }

    bool IProcessRegions::mapped_file(const MemoryRegion &region, estr_t *image_path) const
    {
        return mapped_file(region.base, image_path);
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
        _mapped_file.clear();
    }

    void IProcessRegions::_retrive_memory_regions()
    {
        this->_clear_regions();
        _regions = get_all_memory_regions();
        for (const auto &region : _regions)
        {
            if (region.readable())
                _readable_regions.push_back(region);
            if (region.readable() && region.writable())
                _readwriteable_regions.push_back(region);

            if (region.type == MEM_IMAGE)
            {
                estr_t image_path;
                if (get_mapped_file(region.base, &image_path))
                {
                    auto base_name = file_base_name(image_path);
                    _mapped_file[region.base] = base_name;
                }
            }
        }
    }

    void ProcessAddressTypeJudger::_retrive_memory_informations()
    {
        process_base = _basic_process.base();
        auto process_base_msb = msb(process_base);
        auto memory_type_mask_bit = 4;
        auto memory_all_mask = (1i64 << (process_base_msb)) - 1;
        auto memory_lower_mask = (1i64 << (process_base_msb - memory_type_mask_bit)) - 1;
        memory_type_mask = memory_all_mask - memory_lower_mask;
        process_executable_memory_type_mask = process_base & memory_type_mask;

        estr_t image_path;
        if (_addressable_process.mapped_file(process_base, &image_path))
        {
            auto main_file_name = file_base_name(image_path);
            _main_regions = _addressable_process.file_regions(main_file_name);
        }
    }

    void ProcessAddressTypeJudger::init()
    {
        _retrive_memory_informations();
    }

    MemoryRegions ProcessAddressTypeJudger::main_file_regions() const
    {
        return _main_regions;
    }

    bool ProcessAddressTypeJudger::seems_heap_address(rptr_t address) const
    {
        return address != rnullptr && address < process_base && (address & memory_type_mask) != process_executable_memory_type_mask;
    }

    bool ProcessAddressTypeJudger::seems_executable_address(rptr_t address) const
    {
        return (address & memory_type_mask) == process_executable_memory_type_mask;
        //return address > 0x7f0000000000 && address < 0x7FFF00000000;
    }

    bool ProcessAddressTypeJudger::is_address_seems_near(rptr_t p1, rptr_t p2) const
    {
        if (seems_executable_address(p1))
            return seems_executable_address(p2) && std::abs((int64_t)(p1 - p2)) < 0x80000000;
        else
            return !seems_executable_address(p2) && std::abs((int64_t)(p1 - p2)) < 0x80000000;
    }

}
