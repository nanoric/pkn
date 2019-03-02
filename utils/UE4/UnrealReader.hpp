#pragma once

#include <optional>

#include <pkn/core/reader/TypedReader.hpp>
#include <pkn/core/injector/injector.hpp>
#include "pkn/core/marcos/debug_print.h"

#include "Types/EngineClass.h"


namespace UE4
{
class UnrealReader
{
public:
    UnrealReader() = default;
    ~UnrealReader() = default;
public:
    template <class T>
    std::optional<T> read(erptr_t ptr) const noexcept
    {
        std::aligned_storage_t<sizeof(T)> buffer[1];
        if (tr.read_into(ptr, buffer))
        {
            return *(T *)buffer;
        }
        return std::nullopt;
    }

    template <>
    std::optional<erptr_t> read(erptr_t ptr) const noexcept
    {
        std::aligned_storage_t<sizeof(rptr_t)> buffer[1];
        if (tr.read_into(ptr, buffer))
        {
            return *(rptr_t *)buffer;
        }
        return std::nullopt;
    }

    template <class T>
    std::optional<T> read(void * ptr) const noexcept
    {
        std::aligned_storage_t<sizeof(T)> buffer[1];
        if (tr.read_into(ptr, buffer))
        {
            return *(T *)buffer;
        }
        return std::nullopt;
    }

    template <>
    std::optional<erptr_t> read(void * ptr) const noexcept
    {
        std::aligned_storage_t<sizeof(rptr_t)> buffer[1];
        if (tr.read_into(ptr, buffer))
        {
            return *(rptr_t *)buffer;
        }
        return std::nullopt;
    }

    template <class T>
    std::optional<T> read(T * ptr) const noexcept
    {
        std::aligned_storage_t<sizeof(T)> buffer[1];
        if (tr.read_into(ptr, buffer))
        {
            return *(T *)buffer;
        }
        return std::nullopt;
    }

public:
    pkn::TypedReader &tr = pkn::SingletonInjector<pkn::TypedReader>::get();
};


}
