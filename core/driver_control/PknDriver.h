#pragma once

#include <stdint.h>
#include <Windows.h>

#include <optional>

#include "../base/types.h"
#include "../base/noncopyable.h"
#include "DriverBase.h"
#include <kernel/names.h>


namespace pkn
{
class PknDriver : public DriverBase
{
public:
    PknDriver();
    bool open(const wchar_t *device_name = make_estr(PlayerKnownsDriverDeviceName).to_wstring().c_str()) { return DriverBase::open(device_name); }
public:
    // setter
    inline void set_xor_key(uint64_t xor_key) { this->xor_key = xor_key; }

public:
    // system
    bool query_system_information(uint64_t informaiton_class, void *buffer, uint32_t buffer_size, size_t *ret_size) const noexcept;

    // System process  memory
    std::optional<uint64_t> read_system_memory(erptr_t remote_address, size_t size, void *buffer) const noexcept;
    std::optional<uint64_t> write_system_memory(erptr_t remote_address, size_t size, const void *data) const noexcept;
    std::optional<erptr_t> allocate_nonpaged_memory(size_t size) const noexcept;
    bool free_nonpaged_memory(erptr_t ptr) const noexcept;

    // process memory
    //void read_process_memories(pid_t pid, size_t count, const ReadProcessMemoriesData *pdatas) const;
    bool read_process_memory(const pid_t &pid, const erptr_t &remote_address, size_t size, void *buffer) const noexcept;
    bool write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const noexcept;
    bool acquire_lock(const pid_t &pid, const erptr_t &remote_lock_address) const noexcept;

    // process
    bool wait_for_process(pid_t pid, uint64_t timeout_nanosec, NTSTATUS *status)const noexcept;
    bool wait_for_thread(pid_t tid, uint64_t timeout_nanosec, NTSTATUS *status)const noexcept;
    inline bool is_process_alive(pid_t pid) const noexcept { NTSTATUS status; wait_for_process(pid, 0, &status); return status == STATUS_TIMEOUT; };
    bool force_write_process_memory(pid_t pid, const erptr_t &remote_address, size_t size, const void *data)const noexcept;
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
                            _In_opt_ PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
                            _In_ bool CreateSuspended,
                            _In_opt_ uint64_t MaximumStackSize,
                            _In_ uint64_t CommittedStackSize,
                            _In_ uint64_t StartAddress,
                            _In_opt_ uint64_t Parameter,
                            _Out_opt_ pid_t *out_pid,
                            _Out_opt_ pid_t *tid
    )const noexcept;

    bool allocate_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t type, uint32_t protect, erptr_t *allocated_base, size_t *allocated_size) const noexcept;
    bool free_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t type, erptr_t *freed_base = nullptr, size_t *freed_size = nullptr) const noexcept;
    bool protect_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t protect, erptr_t *protected_base = nullptr, size_t *protected_size = nullptr, uint32_t *old_protect = nullptr) const noexcept;
    std::optional<uint64_t> delete_unloaded_drivers(erptr_t rva_mm_unloaded_drivers, erptr_t rva_mm_last_unloaded_driver, estr_t name_pattern) const noexcept;

    // driver mapping
    std::optional<uint64_t> run_driver_entry(erptr_t entry, uint64_t arg1, uint64_t arg2) const noexcept;
    //std::optional<uint64_t> map_and_run_driver(erptr_t image_buffer,
    //                        euint64_t image_size,
    //                        erptr_t entry_rva,
    //                        erptr_t parameter1,
    //                        erptr_t parameter2,
    //                        erptr_t parameter1_is_rva,
    //                        erptr_t parameter2_is_rva
    //);
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

    // not implemented
private:
    bool _get_physical_memory_address(pid_t pid, erptr_t remote_address, uint64_t *pphysical_address) const noexcept;
    bool _write_physical_memory(erptr_t remote_address, size_t size, const void *data) const noexcept;
    uint64_t xor_key = compile_time::random();
};
}
