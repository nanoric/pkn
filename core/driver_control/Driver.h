#pragma once

#include <Windows.h>
#include <stdint.h>
#include <stdexcept>
#include <mutex>

#include "../base/types.h"
#include "../base/noncopyable.h"

namespace pkn
{
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
        bool read_process_memory(pid_t pid, erptr_t remote_address, size_t size, void *buffer) const noexcept;
        //void read_process_memories(pid_t pid, size_t count, const ReadProcessMemoriesData *pdatas) const;
        bool wait_for_process(pid_t pid, UINT64 timeout_nanosec, NTSTATUS *status)const noexcept;
        bool wait_for_thread(pid_t tid, UINT64 timeout_nanosec, NTSTATUS *status)const noexcept;
        //inline bool is_process_alive2(pid_t pid)const { return get_process_exit_status(pid) != 1; };
        inline bool is_process_alive(pid_t pid)const noexcept { NTSTATUS status; wait_for_process(pid, 0, &status); return status == STATUS_TIMEOUT; };
        bool write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const noexcept;
        bool force_write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data)const noexcept;
        bool virtual_query(pid_t pid, erptr_t remote_address, MEMORY_BASIC_INFORMATION *mbi) const noexcept;
        bool get_mapped_file(pid_t pid, uint64_t address, estr_t *mapped_file) const noexcept;
        bool get_process_base(pid_t pid, erptr_t *base) const noexcept;
        bool get_process_exit_status(pid_t pid, NTSTATUS *status)const noexcept;
        bool get_process_times(pid_t pid, uint64_t * pcreation_time, uint64_t * pexit_time, uint64_t * pkernel_time, uint64_t * puser_time)const noexcept;
        bool get_process_name(pid_t pid, estr_t *name) const noexcept;
        erptr_t get_peb_address() const noexcept;
        bool query_process_information(UINT64 pid, UINT64 informaiton_class, void *buffer, UINT64 buffer_size, ULONG *ret_size) const noexcept;
        bool query_thread_information(UINT64 tid, UINT64 informaiton_class, void *buffer, UINT64 buffer_size, ULONG *ret_size) const noexcept;
        bool create_user_thread(UINT64 pid,
            IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
            IN bool CreateSuspended,
            IN UINT64 MaximumStackSize OPTIONAL,
            IN UINT64 CommittedStackSize OPTIONAL,
            IN UINT64 StartAddress,
            IN UINT64 Parameter OPTIONAL,
            OUT UINT64 *out_pid OPTIONAL,
            OUT UINT64 *tid OPTIONAL
            )const noexcept;


    public:
        // mouse
        bool get_mouse_pos(int *x, int *y) const noexcept;
        /* x : 0-65535, y : 0-65535 */
        //void set_mouse_pos(int x, int y);
        //void synthesize_mouse(SynthesizeMouseData *datas, size_t count);

    public:
        // process protect
        bool protect_process(pid_t pid) const noexcept;
        bool unprotect_process()const  noexcept;

    protected:
        // protected member
        static void xor_memory(void *address, size_t size, uint64_t xor_key) noexcept;
    private:
        bool _get_physical_memory_address(pid_t pid, erptr_t remote_address, uint64_t *pphysical_address) const noexcept;
        bool _write_physical_memory(erptr_t remote_address, size_t size, const void *data) const noexcept;
    private:
        //bool ioctl(uint32_t code, void *input, uint32_t input_size) const;
        bool ioctl(uint32_t code, void *input, uint32_t input_size, void *output, uint32_t *outpu_size) const noexcept;
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
