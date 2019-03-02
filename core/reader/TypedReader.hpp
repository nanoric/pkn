#pragma once

#include <vector>
#include <type_traits>

#include "../base/types.h"
#include "../remote_process/IProcess.h"

namespace pkn
{
    class TypedReader
    {
    public:
        TypedReader(IProcessReader *readable_process) : _readable_process(readable_process) {}
        virtual ~TypedReader() = default;
    public:
    public:
        // custom read rptr_t as type
        template <typename T>
        inline bool read_into(erptr_t remote_address, T *buffer) const noexcept
        {
            return _readable_process->read_unsafe(remote_address, sizeof(T), buffer);
        }

        template <typename T> // T is uintxx_t
        inline bool read_into(erptr_t remote_address, typename encrypted_number<T> *buffer) const noexcept
        {
            T val;
            auto res = _readable_process->read_unsafe(remote_address, sizeof(rptr_t), &val);
            *buffer = val;
            return res;
        }

        template <typename T>
        inline bool read_sequence(erptr_t remote_address, size_t number,T *seq_buffer) const noexcept
        {
            if (number == 0)
                return false;
            return _readable_process->read_unsafe(remote_address, sizeof(T) * number, seq_buffer);
        }

        // custom read rptr_t as type
        template <typename T>
        inline bool read_into(void *remote_address, T *buffer) const noexcept
        {
            return _readable_process->read_unsafe((rptr_t)remote_address, sizeof(T), buffer);
        }

        template <typename T> // T is uintxx_t
        inline bool read_into(void * remote_address, typename encrypted_number<T> *buffer) const noexcept
        {
            T val;
            auto res = _readable_process->read_unsafe((rptr_t)remote_address, sizeof(rptr_t), &val);
            *buffer = val;
            return res;
        }

        template <typename T>
        inline bool read_sequence(void * remote_address, size_t number,T *seq_buffer) const noexcept
        {
            if (number == 0)
                return false;
            return _readable_process->read_unsafe((rptr_t)remote_address, sizeof(T) * number, seq_buffer);
        }
    private:
        IProcessReader *_readable_process;
    };
}

