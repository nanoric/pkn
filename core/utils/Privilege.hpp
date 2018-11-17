#pragma once

#include <Windows.h>

namespace pkn
{
class Privilege
{
public:
    static bool enable_privilege(const char *name)
    {
        TOKEN_PRIVILEGES privilege;
        HANDLE tokenHandle;

        privilege.PrivilegeCount = 1;
        privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (!LookupPrivilegeValueA(NULL, name,
                                   &privilege.Privileges[0].Luid))
            return false;

        if (!OpenProcessToken(GetCurrentProcess(),
                              TOKEN_ADJUST_PRIVILEGES, &tokenHandle))
            return false;

        if (!AdjustTokenPrivileges(tokenHandle,
                                   false,
                                   &privilege,
                                   sizeof(privilege),
                                   nullptr,
                                   nullptr))
        {
            CloseHandle(tokenHandle);
            return false;
        }

        CloseHandle(tokenHandle);
        return true;
    }
};
}
