
#include <Windows.h>
#include <winternl.h>

#include "ProcessUtils.h"

namespace pkn
{
    std::optional<std::vector<pid_t>> ProcessUtils::all_pids() const noexcept
    {
        if (auto infos = all_process_information(); infos)
        {
            std::vector<pid_t> pids;
            for (const auto &p : *infos)
            {
                pids.emplace_back(p.first);
            }
            return pids;
        }
        return std::nullopt;
    }

    std::optional<std::unordered_set<pid_t>> ProcessUtils::all_tids(pid_t pid) const noexcept
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
        return std::nullopt;
    }

    std::optional<std::unordered_map<pid_t, ProcessInfo>> ProcessUtils::all_process_information() const noexcept
    {
        size_t buffer_size = 1024 * 1024; // 1MB buffer should be enough
        std::unique_ptr<char> pbuffer(new char[buffer_size]);

        if (this->query_system_information(0x05,/*SystemProcessInformation = 0x05*/
            pbuffer.get(),
            (uint32_t)buffer_size,
            &buffer_size))
        {
            std::unordered_map<pid_t, ProcessInfo> infos;
            ProcessInfo info;
            char *p = pbuffer.get();
            char *e = p + buffer_size;
            SYSTEM_PROCESS_INFORMATION *spi = (SYSTEM_PROCESS_INFORMATION *)p;
            while (true)
            {
                if (info.pid == 0)
                {
                    info.pid = pid_t((uint64_t)spi->UniqueProcessId);
                    //using namespace std;
                    //info.image_name = estr_t(U"System Idle Process"sv);
                    //info.image_name = make_estr(U"System Idle Process");
                }
                else
                {
                    info.pid = pid_t((uint64_t)spi->UniqueProcessId);
                    //info.image_name = std::wstring(spi->ImageName.Buffer, spi->ImageName.Length / 2);

                    // thread info
                    SYSTEM_THREAD_INFORMATION *sti0 = (SYSTEM_THREAD_INFORMATION *)(p + sizeof(SYSTEM_PROCESS_INFORMATION));
                    for (unsigned i = 0; i < spi->NumberOfThreads; i++)
                    {
                        auto &sti = sti0[i];
                        pid_t tid((uint64_t)sti.ClientId.UniqueThread);
                        auto hint = info.tids.lower_bound(tid);
                        if (hint == info.tids.end() || *hint != tid)
                            info.tids.emplace_hint(hint, std::move(tid));
                    }
                }

                infos.emplace(info.pid, std::move(info));

                if (spi->NextEntryOffset == 0)
                    break;
                spi = (SYSTEM_PROCESS_INFORMATION *)(p += spi->NextEntryOffset);
            }
            return infos;
        }
        return std::nullopt;
    }

    std::optional<pid_t> ProcessUtils::pid_from_process_name(const estr_t &target_process_name) const noexcept
    {
        if (auto pids = all_pids(); pids)
        for (const auto& current_pid : *pids)
        {
            if (current_pid <= 4) // 0 : Idle, 4 : System
                continue;
            if (auto process_name = get_process_name(current_pid); process_name)
            {
                auto current_process_name = file_base_name(*process_name);
                if (target_process_name == current_process_name)
                {
                    return current_pid;
                }
            }
        }
        return std::nullopt;
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
        return NT_SUCCESS(status);
    }

    std::optional<estr_t> ProcessUtils::get_process_name(euint64_t pid) const noexcept
    {
        wchar_t name[10240] = {};
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, (DWORD)pid);
        if (process)
        {
            GetProcessImageFileNameW(process, name, sizeof(name));
            CloseHandle(process);
            return std::wstring(name);
        }
        return std::nullopt;
    }
}
