#pragma once

#include <pkn/core/reader/TypedReader.hpp>
#include "pkn/core/injector/injector.hpp"

#include "Types/EngineClass.h"

#include "UnrealNamesCache.hpp"

namespace UE4
{
struct LocalPropertyInfo
{
public:
    int offset;
    estr_t name;
    estr_t type_name;
    int size;
public:
    //static const LocalPropertyInfo invalid_prop;
public:
    bool operator < (const LocalPropertyInfo &rhs) const noexcept
    {
        return this->offset < rhs.offset;
    }
    bool operator < (uint64_t offset) const noexcept
    {
        return this->offset < offset;
    }
};

struct LocalClass
{
public:
    using LocalPropertyInfos = std::map<int, LocalPropertyInfo>; // ensure sorted
public:
    estr_t name;
    LocalPropertyInfos props;
    erptr_t super = 0;
public:
    inline bool has_property_at_offset(int offset) const noexcept
    {
        return this->props.count(offset) > 0;
    }
    inline const std::optional<LocalPropertyInfo> property_for_offset(int offset) const noexcept
    {
        auto it = props.upper_bound(offset);
        if (it != props.cbegin())
            return (--it)->second;
        return std::nullopt;
    }
    std::optional<LocalClass> get_super()
    {
        return parse(super, false);
    }
public:
    static std::optional<LocalClass> parse(const erptr_t &remote_address, bool with_parents = true)
    {
        auto &unreal_reader = pkn::SingletonInjector<UnrealReader>::get();
        auto &name_cache = pkn::SingletonInjector<UnrealNameCache>::get();
        if (auto res = unreal_reader.read<UClass>(remote_address))
        {
            const UClass &c = *res;
            return parse(c, with_parents);
        }
        return std::nullopt;
    }

    static std::optional<LocalClass> parse(const UClass &c, bool with_parents = true)
    {
        auto &unreal_reader = pkn::SingletonInjector<UnrealReader>::get();
        auto &name_cache = pkn::SingletonInjector<UnrealNameCache>::get();
        if (auto res = name_cache.name_for_object(c))
        {
            auto &cn = *res;
            if (cn == make_estr("Object"))
            {
                LocalClass lc;
                lc.name = cn;
                lc.super = 0;
                return lc;
            }
            LocalPropertyInfos ps;
            UField *children = c.Children;
            while (children != nullptr)
            {
                if (auto res = unreal_reader.read<UProperty>(children))
                {
                    const auto &prop = *res;
                    LocalPropertyInfo lpi;
                    if (prop.Class)
                    {
                        if (auto res = unreal_reader.read<UClass>(prop.Class))
                        {
                            const auto &pc = *res;
                            if (auto pcn = name_cache.name_for_object(pc))
                            {
                                lpi.type_name = *pcn;
                            }
                        }
                    }

                    lpi.name = name_cache.name_for_object(prop).value_or(make_estr("unknown"));
                    lpi.size = prop.ElementSize;
                    lpi.offset = prop.Offset_Internal;
                    ps.emplace(lpi.offset, lpi);
                    children = prop.Next;
                }
                else
                    break;
            }
            LocalClass lc;
            lc.name = cn;
            lc.props = ps;
            lc.super = (uint64_t)c.SuperStruct;
            if (with_parents)
            {
                LocalClass cur = lc;
                if (auto res = cur.get_super())
                {
                    const auto &pprops = res->props;
                    lc.props.insert(pprops.cbegin(), pprops.cend());
                    cur = *res;
                }
            }
            return lc;
        }
        return std::nullopt;
    }

};

//__declspec(selectany) const LocalPropertyInfo LocalPropertyInfo::invalid_prop = LocalPropertyInfo{};

}

