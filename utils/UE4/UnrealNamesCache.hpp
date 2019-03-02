#pragma once

#include <algorithm>
#include <optional>
#include <unordered_map>

#include <pkn/core/base/types.h>
#include <pkn/core/reader/TypedReader.hpp>
#include <pkn/core/injector/injector.hpp>
#include "pkn/core/marcos/debug_print.h"

#include "Types/EngineClass.h"
#include "UnrealReader.hpp"
#include <pkn/core/remote_process/KernelProcess.h>
#include <pkn/core/remote_process/MemoryRegion.h>

namespace UE4
{
class UnrealNameCache
{
public:
    constexpr static const int NAME_WIDE_MASK = 0x01;
    constexpr static const int NAME_INDEX_SHIFT = 1;
public:
    UnrealNameCache(erptr_t ppnames)
        : ppnames(ppnames)
    {}
    bool init()
    {
        return make_name_cache() && make_reverse_cache();
    }
    inline bool make_name_cache()
    {
        auto &kp = pkn::SingletonInjector<pkn::KernelProcess>::get();
        erptr_t pnames;
        if (!tr.read_into(ppnames, &pnames))
            return false;
        if (!tr.read_into(pnames, local_gnames_buffer.get()))
            return false;

        const int index_number_for_one_chunk = TNameEntryArray::ElementsPerChunk;
        auto n = local_gnames_buffer->NumChunks;
        for (int i = 0; i < n; i++)
        {
           auto pns = this->local_gnames_buffer->Chunks[i];
            if (pns != 0)
            {
                std::vector<FNameEntry *>pnames_entry(index_number_for_one_chunk);
                if (this->tr.read_sequence(erptr_t(pns), index_number_for_one_chunk, &pnames_entry[0]))
                {
                    for (size_t j = 0, n = index_number_for_one_chunk; j < n; j++)
                    {
                        auto pname_entry = pnames_entry[j];
                        if (pname_entry != nullptr)
                        {
                            uint32_t index = uint32_t(i * index_number_for_one_chunk + j);
                            FNameEntry name;
                            if (tr.read_into((rptr_t)pname_entry, &name))
                            {
                                _max_index = std::max(_max_index, index);
                                if (name.Index & NAME_WIDE_MASK)
                                {
                                    std::wstring stdname = name.WideName;
                                    _cache.emplace(index, stdname);
                                }
                                else
                                {
                                    std::string stdname = name.AnsiName;
                                    _cache.emplace(index, stdname);
                                }
                            }
                        }
                    }
                }
            }
        }
        _cached = true;
        return true;
    }
    inline bool make_reverse_cache()
    {
        if (!_cached)
        {
            return false;
        }
        for (const auto &p : _cache)
        {
            _reverse_cache.emplace(p.second, p.first);
        }
        return _reverse_cached = true;
    }
    inline bool name_full_cached() const noexcept
    {
        return _cached;
    }
    inline bool is_name_index(int32_t index) const noexcept
    {
        return _cache.count(index) > 0;
    }
public:
    std::optional<estr_t> name_for_object(const UObject &obj) const noexcept
    {
        return name_for_index(obj.Name.ComparisonIndex);
    }
    std::optional<estr_t> name_for_index(int32_t index) const noexcept
    {
        auto it = _cache.find(index);
        if (it != _cache.end())
            return it->second;
        return std::nullopt;
    }
    inline std::optional<int32_t> index_for_name(const estr_t &name) const
    {
        auto it = _reverse_cache.find(name);
        if (it != _reverse_cache.end())
            return it->second;
        return std::nullopt;
    }
private:
    std::unique_ptr<TNameEntryArray> local_gnames_buffer = std::unique_ptr<TNameEntryArray>(new TNameEntryArray);
    erptr_t ppnames;
private:
    uint32_t _max_index;
    bool _cached = false;
    bool _reverse_cached = false;
    std::unordered_map<int32_t, estr_t> _cache = std::unordered_map<int32_t, estr_t>(300000);
    std::unordered_map<estr_t, int32_t> _reverse_cache = std::unordered_map<estr_t, int32_t>(300000);
private:
    pkn::TypedReader &tr = pkn::SingletonInjector<pkn::TypedReader>::get();
};
}
