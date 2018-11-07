#pragma once

#include <stdint.h>
#include "../base/noncopyable.h"

namespace pkn 
{
    class DriverBase : noncopyable
    {
        //const size_t maximun_copy_size_one_piece = 0x7FFFFFFF;
    public:
        DriverBase() = default;;
        ~DriverBase();
        inline bool is_opened() { return _handle != (void *)-1 && _handle != nullptr; };
        // initialization
        bool open(const wchar_t *device_name);
        void close();
    protected:
        //bool ioctl(uint32_t code, void *input, uint32_t input_size) const;
        _Success_(return) bool ioctl(
            uint32_t code,
            _In_reads_bytes_opt_(input_size) void *input,
            _In_ uint32_t input_size,
            _Out_writes_bytes_to_opt_(*output_size, *output_size) void *output,
            __inout_opt uint32_t *output_size
        ) const noexcept;
    private:
        void *_handle = (void *)-1;
    };
}
