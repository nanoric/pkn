#pragma once

#include <Windows.h>
#include <psapi.h>

#include <pkn/stl/vector>
#include <pkn/stl/unordered_map>
#include <pkn/stl/unordered_set>
#include <pkn/stl/optional>
#include <pkn/stl/unique_ptr>

#include "../base/types.h"
#include "../base/fs/fsutils.h"
#include "../base/noncopyable.h"

namespace pkn
{
    class ProcessInfo
    {
    public:
        pid_t pid;
        estr_t image_name;
        stl::unordered_set<pid_t> tids;
    };

    class ModuleInfo
    {
    public:
        erptr_t base;
        euint64_t size;
        estr_t image_name;
    };

    class ProcessUtils : noncopyable
    {
    public:
        virtual ~ProcessUtils() {}
        stl::optional<stl::vector<ModuleInfo>> all_modules() const noexcept;
        // find a module case Insensitive
        stl::optional<ModuleInfo> kernel_modulei(const estr_t &pattern) const noexcept;
        stl::optional<stl::vector<pid_t>> all_pids() const noexcept;
        stl::optional<stl::unordered_set<pid_t>> all_tids(pid_t pid) const noexcept;
        stl::optional<stl::unordered_map<pid_t, ProcessInfo>> all_process_information() const noexcept;
        stl::optional<pid_t> pid_from_process_name(const estr_t &target_process_name) const noexcept;
    protected:
        template <class ReturnType=char>
        stl::unique_ptr<ReturnType[]> system_information(uint64_t informaiton_class) const noexcept;
    protected:
        virtual bool query_system_information(uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept;
        virtual stl::optional<estr_t> get_process_name(euint64_t pid) const noexcept;
    };

    template <class ReturnType/*=char*/>
    stl::unique_ptr<ReturnType[]>
        pkn::ProcessUtils::system_information(uint64_t informaiton_class) const noexcept
    {
        size_t buffer_size = 0;
        if (this->query_system_information(informaiton_class,
            nullptr,
            0,
            &buffer_size))
        {
            stl::unique_ptr<ReturnType[]> pbuffer(new char[buffer_size]);

            if (this->query_system_information(informaiton_class,
                pbuffer.get(),
                (uint32_t)buffer_size,
                &buffer_size))
            {
                return pbuffer;
            }
        }
        return nullptr;
    }

}

