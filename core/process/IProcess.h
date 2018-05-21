#pragma once

#include "../base/types.h"

#include "../base/abstract/abstract.h"

namespace pkn
{
    class IBasicProcess
    {
    public:
        virtual ~IBasicProcess() = default;
    public:
        virtual erptr_t base() const PURE_VIRTUAL_FUNCTION_BODY;
        virtual bool alive() const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IReadableProcess
    {
    public:
        virtual ~IReadableProcess() = default;
    public:
        virtual bool read_unsafe(erptr_t address, size_t size, void *buffer) const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IWritableProcess
    {
    public:
        virtual ~IWritableProcess() = default;
    public:
        virtual bool write_unsafe(erptr_t address, size_t size, const void *buffer) const PURE_VIRTUAL_FUNCTION_BODY;
    };

    class IExtraProcess
    {
    public:
        virtual ~IExtraProcess() = default;
    public:
        virtual erptr_t get_peb_address() const PURE_VIRTUAL_FUNCTION_BODY;
    };
}
