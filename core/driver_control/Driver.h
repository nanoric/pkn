#pragma once

#include <Windows.h>
#include <stdint.h>
#include <stdexcept>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

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
        // system
        bool query_system_information(uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept;
        // new style is enabled : use std::optional instead of bool with pointer
        //std::optional<std::unordered_map<pid_t, ProcessInfo>> get_all_process_information() const noexcept;

        // process
        bool read_process_memory(pid_t pid, erptr_t remote_address, size_t size, void *buffer) const noexcept;
        //void read_process_memories(pid_t pid, size_t count, const ReadProcessMemoriesData *pdatas) const;
        bool wait_for_process(pid_t pid, uint64_t timeout_nanosec, NTSTATUS *status)const noexcept;
        bool wait_for_thread(pid_t tid, uint64_t timeout_nanosec, NTSTATUS *status)const noexcept;
        inline bool is_process_alive(pid_t pid) const noexcept { NTSTATUS status; wait_for_process(pid, 0, &status); return status == STATUS_TIMEOUT; };
        bool write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const noexcept;
        bool force_write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data)const noexcept;
        bool virtual_query(pid_t pid, erptr_t remote_address, MEMORY_BASIC_INFORMATION *mbi) const noexcept;
        bool get_mapped_file(pid_t pid, uint64_t address, estr_t *mapped_file) const noexcept;
        bool get_process_base(pid_t pid, erptr_t *base) const noexcept;
        bool get_process_exit_status(pid_t pid, NTSTATUS *status)const noexcept;
        bool get_process_times(pid_t pid, uint64_t * pcreation_time, uint64_t * pexit_time, uint64_t * pkernel_time, uint64_t * puser_time)const noexcept;
        std::optional<estr_t> get_process_name(pid_t pid) const noexcept;
        erptr_t get_peb_address() const noexcept;
        bool query_process_information(uint64_t pid, uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept;
        bool query_thread_information(uint64_t tid, uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept;
        std::optional<erptr_t> get_teb_address(uint64_t tid);
        bool create_user_thread(pid_t pid,
            IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
            IN bool CreateSuspended,
            IN uint64_t MaximumStackSize OPTIONAL,
            IN uint64_t CommittedStackSize OPTIONAL,
            IN uint64_t StartAddress,
            IN uint64_t Parameter OPTIONAL,
            OUT pid_t *out_pid OPTIONAL,
            OUT pid_t *tid OPTIONAL
            )const noexcept;

        bool allocate_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t type, uint32_t protect, erptr_t *allocated_base, size_t *allocated_size);
        bool free_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t type, erptr_t *freed_base = nullptr, size_t *freed_size = nullptr);
        bool protect_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t protect, erptr_t *protected_base = nullptr, size_t *protected_size = nullptr, uint32_t *old_protect = nullptr);

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
        bool ioctl(__in uint32_t code,
            __in void *input,
            __in uint32_t input_size,
            __in_opt void *output,
            __inout_opt uint32_t *outpu_size) const noexcept;
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
