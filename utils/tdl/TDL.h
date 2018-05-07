#pragma once

#include <string>

extern "C"     int IsVBoxInstalled(
    void
);
extern "C" UINT TDLProcessCommandLine(
    _In_ LPCWSTR lpCommandLine
);

class TDL
{
public:
    TDL()
        = default;
public:
    bool load(const std::wstring &path)
    {
        std::wstring cmd_line = L"program ";
        cmd_line += path;
        return 0 == TDLProcessCommandLine(cmd_line.c_str());
    }
};