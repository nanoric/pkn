#pragma once

#include <Windows.h>
#include <psapi.h>

#include <vector>
#include <mutex>

#include "../base/types.h"
#include "../base/fs/fsutils.h"
#include "../base/noncopyable.h"

namespace pkn
{

    class ProcessUtils : noncopyable
    {
    public:
        virtual ~ProcessUtils() {}

        static std::vector<euint64_t> all_pids()
        {
            DWORD ids[1024];
            DWORD sizeNeeded;
            EnumProcesses(ids, sizeof(ids), &sizeNeeded);
            int number = sizeNeeded / sizeof(DWORD);

            std::vector < euint64_t > idVector(number);
            for (int i = 0; i < number; i++)
            {
                idVector.at(i) = ids[i];
            }
            return idVector;
        }

        bool pid_from_process_name(const estr_t &process_name, pid_t *pid) const noexcept
        {
            for (const auto& current_pid : all_pids())
            {
                if (current_pid <= 4) // 0 : Idle, 4 : System
                    continue;
                estr_t process_name;
                if (get_process_name(current_pid, &process_name))
                {
                    auto current_process_name = file_base_name(process_name);
                    if (process_name == current_process_name)
                    {
                        *pid = current_pid;
                        return true;
                    }
                }
                return false;

            }
            return false;
            //throw std::runtime_error("No process target found");
        }
    protected:
        virtual bool get_process_name(euint64_t pid, estr_t *process_name) const noexcept
        {
            wchar_t name[10240] = {};
            HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, (DWORD)pid);
            if (process)
            {
                GetProcessImageFileNameW(process, name, sizeof(name));
                CloseHandle(process);
                *process_name = std::wstring(name);
                return true;
            }
            return false;
        };
    };
}

