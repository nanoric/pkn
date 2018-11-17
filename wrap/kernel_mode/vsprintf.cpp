#include <vadefs.h>
#include <stdarg.h>
#include <stdio.h>
#include <corecrt.h>

extern "C"
{

    _Success_(return >= 0)
        _Check_return_opt_
        _ACRTIMP int __cdecl __stdio_common_vswprintf(
            _In_                                    unsigned __int64 _Options,
            _Out_writes_opt_z_(_BufferCount)        wchar_t*         _Buffer,
            _In_                                    size_t           _BufferCount,
            _In_z_ _Printf_format_string_params_(2) wchar_t const*   _Format,
            _In_opt_                                _locale_t        _Locale,
            va_list          _ArgList
        )
    {
        _Options;
        _Buffer;
        _BufferCount;
        _Format;
        _Locale;
        _ArgList;
        return 0;
    }
    _Success_(return >= 0)
        _ACRTIMP int __cdecl __stdio_common_vsprintf(
            _In_                                    unsigned __int64 _Options,
            _Out_writes_opt_z_(_BufferCount)        char*            _Buffer,
            _In_                                    size_t           _BufferCount,
            _In_z_ _Printf_format_string_params_(2) char const*      _Format,
            _In_opt_                                _locale_t        _Locale,
            va_list          _ArgList
        )
    {
        _Options;
        _Buffer;
        _BufferCount;
        _Format;
        _Locale;
        _ArgList;
        return 0;
    }

//    int __stdio_common_vswprintf(char *str, const char *format, ...)
//    {
//        va_list va;
//        va_start(va, format);
//        return vsprintf(str, format, va);
//}
}
