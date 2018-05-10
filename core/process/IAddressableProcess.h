#pragma once
#pragma once

#include <unordered_map>

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
        virtual estr_t get_mapped_file(erptr_t remote_address) const PURE_VIRTUAL_FUNCTION_BODY;
    public:
        void refresh_regions();
    public:
        MemoryRegions file_regions(const estr_t &executable_name) const;

        MemoryRegions memory_regions() const;
        MemoryRegions readable_regions() const;
        MemoryRegions readwritable_regions() const;
        estr_t mapped_file(erptr_t remote_address) const;
        estr_t mapped_file(const MemoryRegion &region) const;
    private:
        void _clear_regions();
        void _retrive_memory_regions();
    private:
        MemoryRegions _regions;
        MemoryRegions _readable_regions;
        MemoryRegions _readwriteable_regions;
        std::unordered_map<erptr_t, estr_t> _mapped_file;
    };

    class ProcessAddressTypeJudger
    {
    public:
        // this need a instance both inherit IProcessRegions and IBasicProcess
        template <class T>
        ProcessAddressTypeJudger(T &process) : _addressable_process(process), _basic_process(process) {}
        virtual ~ProcessAddressTypeJudger() = default;
    protected:
        void init();
    public:
        MemoryRegions main_file_regions() const;
        bool seems_heap_address(erptr_t address) const;
        bool seems_executable_address(erptr_t address) const;

        // address differ less than 2GB
        bool is_address_seems_near(erptr_t p1, erptr_t p2) const;
    private:
        void _retrive_memory_informations();
    private:
        MemoryRegions _main_regions;
        rptr_t process_executable_memory_type_mask;
        rptr_t memory_type_mask;
        IBasicProcess &_basic_process;
        IProcessRegions &_addressable_process;
    };
}

