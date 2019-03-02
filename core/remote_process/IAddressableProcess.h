#pragma once
#pragma once

#include <unordered_map>
#include <optional>

#include "../base/types.h"
#include "../base/abstract/abstract.h"

#include "MemoryRegion.h"
#include "IProcess.h"

namespace pkn
{

class IProcessRegions
{
public:
    virtual ~IProcessRegions() = default;
protected:
    void init();
    virtual MemoryRegions get_all_memory_regions() PURE_VIRTUAL_FUNCTION_BODY;
    virtual bool get_mapped_file(erptr_t remote_address, estr_t *out_mapped_file) const PURE_VIRTUAL_FUNCTION_BODY;
public:
    void refresh_regions();
public:

    // case sensitive version
    MemoryRegions file_regions(const estr_t &executable_name) const;

    // case insensitive version
    MemoryRegions file_regionsi(const estr_t &executable_name) const;

    const MemoryRegions &memory_regions() const;
    const MemoryRegions &readable_regions() const;
    const MemoryRegions &readwritable_regions() const;
    const MemoryRegions &readexecutable_regions() const;
    const MemoryRegions &readwritexecutable_regions() const;

    /*
    return mapped file for a specific address, this address must be the base address of mapped section
    time complexity: O(log2(n))
    */
    std::optional<estr_t> mapped_file_for_address(const erptr_t &remote_address) const;

    /*
    return mapped file for a specific base address, this base address must be the base address of mapped section
    time complexity: O(1)
    */
    std::optional<estr_t> mapped_file_for_base(const erptr_t &remote_base_address) const;

    /*
    return mapped file for a specific memory region
    time complexity: O(1)
    */
    std::optional<estr_t> mapped_file(const MemoryRegion &region) const;

    std::optional<MemoryRegion> region_for_address(const erptr_t &remote_address) const;
private:
    void _clear_regions();
    void _retrive_memory_regions();
private:
    MemoryRegions _regions;
    MemoryRegions _readable_regions;
    MemoryRegions _readwriteable_regions;
    MemoryRegions _readexecutable_regions;
    MemoryRegions _readwritexecutable_regions;
    std::unordered_map<erptr_t, estr_t> _mapped_file;
};

class ProcessAddressTypeInfo
{
public:
    // this need a instance both inherit IProcessRegions and IBasicProcess
    ProcessAddressTypeInfo() = default;
    virtual ~ProcessAddressTypeInfo() = default;
protected:
    void init(IProcessBasic *_basic_process, IProcessRegions *_addressable_process);
public:
    MemoryRegions main_file_regions() const;
    bool seems_heap_address(rptr_t address) const;
    bool seems_executable_address(rptr_t address) const;

    // address differ less than 2GB
    bool is_address_seems_near(rptr_t p1, rptr_t p2) const;
private:
    void _retrive_memory_informations(IProcessBasic *_basic_process, IProcessRegions *_addressable_process);
private:
    MemoryRegions _main_regions;
    erptr_t process_base;
    rptr_t process_executable_memory_type_mask;
    rptr_t memory_type_mask;
};
}

