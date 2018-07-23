#pragma once

#include <Windows.h>
#include <psapi.h>

#include <vector>
#include <unordered_set>
#include <mutex>
#include <optional>

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
        std::unordered_set<pid_t> tids;
    };

    class ProcessUtils : noncopyable
    {
    public:
        virtual ~ProcessUtils() {}
        std::optional<std::vector<pid_t>> all_pids() const noexcept;
        std::optional<std::unordered_set<pid_t>> all_tids(pid_t pid) const noexcept;
        std::optional<std::unordered_map<pid_t, ProcessInfo>> all_process_information() const noexcept;
        std::optional<pid_t> pid_from_process_name(const estr_t &target_process_name) const noexcept;
    protected:
        virtual bool query_system_information(uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept;
        virtual std::optional<estr_t> get_process_name(euint64_t pid) const noexcept;
    };
}

