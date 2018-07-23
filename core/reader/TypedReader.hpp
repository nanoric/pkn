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
        inline bool read(erptr_t remote_address, T *buffer) const
        {
            return _readable_process.read_unsafe(remote_address, sizeof(T), buffer);
        }

        template <typename T>
        inline bool read(erptr_t remote_address, typename encrypted_number<T> *buffer) const
        {
            rptr_t val;
            auto res = _readable_process.read_unsafe(remote_address, sizeof(rptr_t), &val);
            *buffer = val;
            return res;
        }

        template <typename T>
        inline bool read_sequence(erptr_t remote_address, size_t number,T *seq_buffer) const
        {
            if (number == 0)
                return false;
            return _readable_process.read_unsafe((rptr_t)remote_address, sizeof(T) * number, seq_buffer);
        }
    private:
        IReadableProcess & _readable_process;
    };
}

