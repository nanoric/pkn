#pragma once

#include "../../core/base/types.h"
#include <filesystem>
#include <unordered_map>
#include <optional>

#include "../../core/base/types.h"
#include "dia/dia2.h"

#if defined(_AMD64_)
#pragma comment(lib, "diaguids64.lib")
#else
#pragma comment(lib, "diaguids.lib")
#endif


namespace pkn
{
class PdbHelper
{
public:
    PdbHelper();
    ~PdbHelper();
public:
    bool init(const estr_t &filepath, const estr_t &local_cache_path);

    std::optional<euint32_t> symbol_address(const estr_t &symbol_name)
    {
        if (auto it = _cache.find(symbol_name); it != _cache.end())
        {
            return it->second;
        }
        return std::nullopt;
    }
private:
    IDiaDataSource *create_ida_source();
    bool populate_symbols();
private:
    // make the compiler happy
    static estr_t make_symbol_path(const std::wstring &local_cache_path) noexcept
    {
        return
        estr_t(make_estr("srv*").to_wstring()) +
        estr_t(local_cache_path) +
        estr_t(make_estr("*https://msdl.microsoft.com/download/symbols"));
    }

    // make the compiler happy
    static auto make_DllGetClassObject() noexcept
    {
        return make_const_encrypted_string("DllGetClassObject");
    }
private:
    IDiaDataSource *_source = nullptr;
    IDiaSession *_session = nullptr;
    IDiaSymbol *_global = nullptr;

    std::unordered_map<estr_t, euint32_t> _cache;      // Symbol name <--> RVA map
};
}
