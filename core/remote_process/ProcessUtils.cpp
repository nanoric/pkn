
#include <Windows.h>
#include <winternl.h>

#include "ProcessUtils.h"

constexpr uint32_t SystemModuleInformation = 0x0b;
constexpr NTSTATUS STATUS_INFO_LENGTH_MISMATCH = 0xC0000004;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    CHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

namespace pkn
{
stl::optional<stl::vector<ModuleInfo>> ProcessUtils::all_modules() const noexcept
{
    if (auto ptr = this->system_information(SystemModuleInformation); ptr)
    {
        auto pinfo = (PRTL_PROCESS_MODULES)ptr.get();
        auto n = pinfo->NumberOfModules;
        stl::vector<pkn::ModuleInfo> infos;
        infos.reserve(n);
        for (uint32_t i = 0; i < n; i++)
        {
            RTL_PROCESS_MODULE_INFORMATION &entry = pinfo->Modules[i];
            ModuleInfo mi;
            mi.base = (rptr_t)entry.ImageBase;
            mi.size = (rptr_t)entry.ImageSize;
            mi.image_name = stl::string(entry.FullPathName);
            infos.emplace_back(stl::move(mi));
        }
        return infos;
    }
    return stl::nullopt;
}

stl::optional<ModuleInfo> ProcessUtils::kernel_modulei(const estr_t &pattern) const noexcept
{
    auto lower_pattern = pattern.to_lower();
    if (auto all = this->all_modules(); all)
    {
        for (const auto &m : *all)
        {
            if (m.image_name.npos != m.image_name.to_lower().find(lower_pattern))
            {
                return m;
            }
        }
    }
    return stl::nullopt;
}

stl::optional<stl::vector<pid_t>> ProcessUtils::all_pids() const noexcept
{
    if (auto infos = all_process_information(); infos)
    {
        stl::vector<pid_t> pids;
        for (const auto &p : *infos)
        {
            pids.emplace_back(p.first);
        }
        return pids;
    }
    return stl::nullopt;
}

stl::optional<stl::unordered_set<pid_t>> ProcessUtils::all_tids(pid_t pid) const noexcept
{
    if (auto infos = all_process_information(); infos)
    {
        for (const auto &p : *infos)
        {
            if (p.first == pid)
            {
                return p.second.tids;
            }
        }
    }
    return stl::nullopt;
}

stl::optional<stl::unordered_map<pid_t, ProcessInfo>> ProcessUtils::all_process_information() const noexcept
{
    if (auto ptr = this->system_information(SystemProcessInformation); ptr)
    {
        auto p = ptr.get();
        SYSTEM_PROCESS_INFORMATION *spi = (SYSTEM_PROCESS_INFORMATION *)p;
        stl::unordered_map<pid_t, ProcessInfo> infos;
        while (true)
        {
            ProcessInfo info;
            if (spi->UniqueProcessId == 0)
            {
                info.pid = pid_t((uint64_t)spi->UniqueProcessId);
                //using namespace stl;
                //info.image_name = make_estr("System Idle Process");
            }
            else
            {
                info.pid = pid_t((uint64_t)spi->UniqueProcessId);
                //info.image_name = stl::wstring(spi->ImageName.Buffer, spi->ImageName.Length / 2);

                // thread info
                SYSTEM_THREAD_INFORMATION *sti0 = (SYSTEM_THREAD_INFORMATION *)(p + sizeof(SYSTEM_PROCESS_INFORMATION));
                for (unsigned i = 0; i < spi->NumberOfThreads; i++)
                {
                    auto &sti = sti0[i];
                    pid_t tid((uint64_t)sti.ClientId.UniqueThread);
                    auto hint = info.tids.find(tid);
                    if (hint == info.tids.end() || *hint != tid)
                        info.tids.emplace_hint(hint, stl::move(tid));
                }
            }

            infos.emplace(info.pid, stl::move(info));

            if (spi->NextEntryOffset == 0)
                break;
            spi = (SYSTEM_PROCESS_INFORMATION *)(p += spi->NextEntryOffset);
        }
        return infos;
    }
    return stl::nullopt;
}

stl::optional<pid_t> ProcessUtils::pid_from_process_name(const estr_t &target_process_name) const noexcept
{
    if (target_process_name == make_hash("Idle"))
    {
        return pid_t(0);
    }
    if (target_process_name == make_hash("System"))
    {
        return pid_t(0);
    }
    if (auto pids = all_pids(); pids)
    {
        for (const auto& current_pid : *pids)
        {
            if (current_pid <= 4) // 0 : Idle, 4 : System
                continue;
            if (auto process_name = this->get_process_name(current_pid); process_name)
            {
                auto current_process_name = file_base_name(*process_name);
                if (target_process_name == current_process_name)
                {
                    return current_pid;
                }
            }
        }
    }
    return stl::nullopt;
}

bool ProcessUtils::query_system_information(uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept
{
    using fZwQuerySystemInformation = NTSTATUS
    (*)(
        IN UINT64 SystemInformationClass,
        OUT PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        OUT PULONG ReturnLength OPTIONAL
        );
    static fZwQuerySystemInformation ZwQuerySystemInformation = (fZwQuerySystemInformation)GetProcAddress(LoadLibraryA("NtDll"), "ZwQuerySystemInformation");
    ULONG os = 0;
    NTSTATUS status = ZwQuerySystemInformation(informaiton_class, buffer, buffer_size, &os);
    *ret_size = os;
    if (buffer == nullptr && status == STATUS_INFO_LENGTH_MISMATCH)
        return true;
    return NT_SUCCESS(status);
}

stl::optional<estr_t> ProcessUtils::get_process_name(euint64_t pid) const noexcept
{
    wchar_t name[1024] = {};
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, (DWORD)pid);
    if (process)
    {
        GetProcessImageFileNameW(process, name, sizeof(name) / sizeof(*name));
        CloseHandle(process);
        return estr_t(name, wcslen(name));
    }
    return stl::nullopt;
}
}
