#pragma once


extern "C"
{
    int IsVBoxInstalled(
        void
    );
    int DsefixProcessCommandLine(
        const wchar_t *lpCommandLine
    );
}


class Dsefix
{
public:
    Dsefix()
        = default;
    ~Dsefix()
    {
        if (disabled)
        {
            enable();
        }
    }
public:
    bool disable()
    {
        bool retv = DsefixProcessCommandLine(L"fix");
        disabled = true;
        return retv;
    }
    bool enable()
    {
        return DsefixProcessCommandLine(L"fix -e");
    }
public:
    bool disabled = false;
};