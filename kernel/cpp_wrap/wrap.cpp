#include <pkn/wrap/kernel_mode/all.cpp>
//extern "C" void __cdecl
//_invalid_parameter(
//    wchar_t const* const expression,
//    wchar_t const* const function_name,
//    wchar_t const* const file_name,
//    unsigned int   const line_number,
//    uintptr_t      const reserved)
//{
//}

extern "C" void __cdecl
_invalid_parameter_noinfo(void) {}

extern "C" __declspec(noreturn) void __cdecl
_invalid_parameter_noinfo_noreturn(void) {}

//extern "C" __declspec(noreturn) void __cdecl
//_invoke_watson(
//    wchar_t const* const expression,
//    wchar_t const* const function_name,
//    wchar_t const* const file_name,
//    unsigned int   const line_number,
//    uintptr_t      const reserved)
//{}
//
void __cdecl std::_Xlength_error(char const *)
{}