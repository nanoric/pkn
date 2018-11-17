#include <Windows.h>

#include "PdbHelper.h"

namespace pkn
{

PdbHelper::PdbHelper()
{}

PdbHelper::~PdbHelper()
{
    // Must be released before CoUninitialize
    if (_global)
        _global->Release();
    if (_session)
        _session->Release();
    if (_source)
        _source->Release();
    CoUninitialize();
}

bool PdbHelper::init(const estr_t &filepath, const estr_t &local_cache_path)
{
    HRESULT hr = CoInitialize(nullptr);
    if (S_OK != hr && S_FALSE != hr && RPC_E_CHANGED_MODE != hr)
        return false;

    auto full_path = std::filesystem::absolute(local_cache_path.to_wstring().c_str());
    if (!std::filesystem::exists(full_path))
        if (!std::filesystem::create_directory(full_path))
            return false;

    auto symbol_path = this->make_symbol_path(full_path.wstring());

    // dia source
    _source = this->create_ida_source();
    if (nullptr == _source)
        return false;

    // load pdb
    hr = _source->loadDataForExe(filepath.to_wstring().c_str(), symbol_path.to_wstring().c_str(), nullptr);
    if (S_OK != hr)
        return false;
    hr = _source->openSession(&_session);
    if (S_OK != hr)
        return false;
    hr = _session->get_globalScope(&_global);
    if (S_OK != hr)
        return false;

    if (!this->populate_symbols())
        return false;

    return true;
}

IDiaDataSource *PdbHelper::create_ida_source()
{
    IDiaDataSource *source = nullptr;
    // Try to get from COM
    HRESULT hr = CoCreateInstance(CLSID_DiaSource,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  __uuidof(IDiaDataSource),
                                  reinterpret_cast<void **>(&_source)
    );

    if (hr == REGDB_E_CLASSNOTREG)
    {
        // Retry with direct export call

        // load library
        HMODULE msdia140 = LoadLibraryW(L"msdia140.dll");
        if (!msdia140)
            return false;

        // get DllGetClassObject
        using fGetClassObject = HRESULT(WINAPI*)(REFCLSID, REFIID, void **);

        auto proc_name = make_DllGetClassObject();
        auto DllGetClassObject = reinterpret_cast<fGetClassObject>(
            GetProcAddress(msdia140, proc_name.to_string().c_str()));

        if (!DllGetClassObject)
            return false;

        // get IClassFactory
        IClassFactory *classFactory;
        hr = DllGetClassObject(CLSID_DiaSource, IID_IClassFactory, reinterpret_cast<void **>(&classFactory));
        if (S_OK != hr)
            return false;

        // create instance
        HRESULT hr = classFactory->CreateInstance(nullptr, IID_IDiaDataSource, reinterpret_cast<void **>(&source));

        return source;
    }
    return source;
}

bool PdbHelper::populate_symbols()
{
    IDiaEnumSymbols *enumurator;
    auto hr = _global->findChildren(SymTagNull, nullptr, nsNone, &enumurator);
    if (S_OK != hr)
        return false;

    ULONG count = 0;
    IDiaSymbol *isymbol;
    while (S_OK == (enumurator->Next(1, &isymbol, &count)) && count != 0)
    {
        DWORD rva = 0;
        wchar_t *name = nullptr;

        isymbol->get_relativeVirtualAddress(&rva);
        isymbol->get_undecoratedName(&name);

        if (name && rva)
        {
            std::wstring wname(name);

            // Remove x86 __stdcall decoration
            if (wname[0] == L'@' || wname[0] == L'_')
            {
                wname.erase(0, 1);
            }

            auto pos = wname.rfind(L'@');
            if (pos != wname.npos)
            {
                wname.erase(pos);
            }

            _cache.emplace(wname, rva);
        }

        isymbol->Release();
    }
    enumurator->Release();
    return true;
}
}
