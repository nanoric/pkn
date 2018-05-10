#pragma once

#include <Windows.h>
#include <stdint.h>
#include <stdexcept>
#include <mutex>

#include "../base/types.h"
#include "../base/noncopyable.h"

namespace pkn
{

    class kernel_open_error : public std::exception
    {};

    class kernel_ioctrl_error : public std::exception
    {};

    class kernel_read_memory_error : public kernel_ioctrl_error
    {};

    class kernel_write_memory_error : public kernel_ioctrl_error
    {};

    class kernel_get_physical_address_error : public kernel_ioctrl_error
    {};

    class kernel_write_physical_memory_error : public kernel_ioctrl_error
    {};

    class kernel_query_virtual_memory_error : public kernel_ioctrl_error
    {};

    class kernel_get_mapped_file_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_name_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_base_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_times_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_exit_status_error : public kernel_ioctrl_error
    {};

    class kernel_get_mouse_pos_error : public kernel_ioctrl_error
    {};

    class kernel_synthesize_mouse_error : public kernel_ioctrl_error
    {};

    class kernel_wait_process_error : public kernel_ioctrl_error
    {};

    class kernel_not_implemented_error : public kernel_ioctrl_error
    {};

    using pid_t = euint64_t;
    class Driver : noncopyable
    {
        //const size_t maximun_copy_size_one_piece = 0x7FFFFFFF;
    public:
        Driver() {};
        ~Driver();
        inline bool is_opened() { return _handle != (void *)-1 && _handle != nullptr; };
    protected:
        // initialization
        bool open(const wchar_t *device_name);

    public:
        // setter
        inline void set_xor_key(uint64_t xor_key) { this->xor_key = xor_key; }

    public:
        // process
        void read_process_memory(pid_t pid, erptr_t remote_address, size_t size, void *buffer) const;
        //void read_process_memories(pid_t pid, size_t count, const ReadProcessMemoriesData *pdatas) const;
        NTSTATUS wait_for_process(pid_t pid)const;
        //inline bool is_process_alive2(pid_t pid)const { return get_process_exit_status(pid) != 1; };
        inline bool is_process_alive(pid_t pid)const { return wait_for_process(pid) == STATUS_TIMEOUT; };
        void write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const;
        void force_write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data)const;
        void virtual_query(pid_t pid, erptr_t remote_address, MEMORY_BASIC_INFORMATION *mbi) const;
        estr_t get_mapped_file(pid_t pid, uint64_t address);
        erptr_t get_process_base(pid_t pid);
        NTSTATUS get_process_exit_status(pid_t pid)const;
        void get_process_times(pid_t pid, uint64_t * pcreation_time, uint64_t * pexit_time, uint64_t * pkernel_time, uint64_t * puser_time);
        estr_t get_process_name(pid_t pid);

    public:
        // mouse
        void get_mouse_pos(int *x, int *y);
        /* x : 0-65535, y : 0-65535 */
        //void set_mouse_pos(int x, int y);
        //void synthesize_mouse(SynthesizeMouseData *datas, size_t count);

    public:
        // process protect
        bool protect_process(pid_t pid);
        void unprotect_process();

    protected:
        // protected member
        static void xor_memory(void *address, size_t size, uint64_t xor_key);
    private:
        bool _get_physical_memory_address(pid_t pid, erptr_t remote_address, uint64_t *pphysical_address) const;
        bool _write_physical_memory(erptr_t remote_address, size_t size, const void *data) const;
    private:
        //bool ioctl(uint32_t code, void *input, uint32_t input_size) const;
        bool ioctl(uint32_t code, void *input, uint32_t input_size, void *output, uint32_t *outpu_size) const;
    private:
        void *_handle = (void *)-1;
        uint64_t xor_key = compile_time::random();
    };

    class PknDriver : public Driver
    {
    public:
        PknDriver();
    };
}
