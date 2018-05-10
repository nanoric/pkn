#pragma once

#include <vector>
#include <type_traits>

#include "../base/types.h"
#include "../process/IProcess.h"

namespace pkn
{
    class TypedReader
    {
    public:
        TypedReader(IReadableProcess &readable_process) : _readable_process(readable_process) {}
        virtual ~TypedReader() = default;
    public:
    public:
        // custom read rptr_t as type
        template <typename T>
        inline T read(erptr_t remote_address) const
        {
            std::aligned_storage<sizeof(T), 16>::type buffer[1];
            _readable_process.read_unsafe(remote_address, sizeof(T), buffer);
            return *(T*)buffer;
        }

        template <typename T>
        inline std::vector<T> read_sequence(const T *remote_address, size_t number) const
        {
            std::vector<T> items(number);
            if (number == 0)
                return items;
            _readable_process.read_unsafe((rptr_t)remote_address, sizeof(T) * number, &items[0]);
            return items;
        }
    private:
        IReadableProcess & _readable_process;
    };
}

