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

        euint64_t pid_from_process_name(const estr_t &process_name)
        {
            for (auto pid : all_pids())
            {
                if (pid <= 4) // 0 : Idle, 4 : System
                    continue;
                try
                {
                    auto current_process_name = file_base_name(get_process_name(pid));
                    if (process_name == current_process_name)
                        return pid;
                }
                catch (const std::exception&)
                {
                }
            }
            throw std::runtime_error("No process target found");
        }
    protected:
        virtual estr_t get_process_name(euint64_t pid)
        {
            wchar_t name[10240] = {};
            HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, (DWORD)pid);
            if (process)
            {
                GetProcessImageFileNameW(process, name, sizeof(name));
                CloseHandle(process);
            }
            std::wstring sname(name);
            return std::u32string(sname.begin(), sname.end());
        };
    };
}

