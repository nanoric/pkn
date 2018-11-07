#include <Windows.h>
#include <winternl.h>

void RtlInitUnicodeString(
    PUNICODE_STRING         DestinationString,
    PCWSTR SourceString
)
{
    auto len = wcslen(SourceString);
    DestinationString->Buffer = (PWSTR)SourceString;
    DestinationString->Length = (USHORT)(len * sizeof(wchar_t));
    DestinationString->MaximumLength = (USHORT)(len + 1 * sizeof(wchar_t));
}


