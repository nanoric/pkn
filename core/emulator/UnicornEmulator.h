#pragma once

#include "../base/types.h"
#include "../process/IProcess.h"

struct uc_struct;
using uc_engine = uc_struct;
enum uc_mem_type;
enum uc_err;
using uc_hook = size_t;

namespace pkn
{

    // only some usually used field are list here
    struct Context
    {
        uint64_t rax;
        uint64_t rbx;
        uint64_t rcx;
        uint64_t rdx;
        uint64_t r8;
        uint64_t r9;
        uint64_t r10;
        uint64_t r11;
        uint64_t r12;
        uint64_t rsp;
        uint64_t rbp;
        uint64_t rsi;
        uint64_t rdi;
        uint64_t rip;
        uint128_t xmm0;
        uint128_t xmm1;
        uint128_t xmm2;
        uint128_t xmm3;
        uint32_t eflags;
    };

    struct Memory
    {
        uint64_t base;
        uint64_t size;
        bool copy = true;
        void *buffer;
    };

    struct EntryParameter
    {
        Context context;
        Memory *memories;
        size_t memories_number;
    };

    class UnicornEmulator
    {
#define no_error_or_ret(err, ret_val) if(UC_ERR_OK != err)  return ret_val;
    public:
        UnicornEmulator(IBasicProcess &basic_process, IReadableProcess &readable_process, IExtraProcess &extra_process, pid_t tid);
        ~UnicornEmulator();
        bool init();

        // this function can be called before every run(), to emulate different thread in target process
        // set tid to 0 to disable the set of gs(improve alittle speed)
        bool set_thread_id(pid_t tid);
    public:
        Context run(erptr_t start_addr, erptr_t end_addr, const EntryParameter &context);
    public:
        inline erptr_t base() const { return basic_process.base(); }
        uint64_t upper_bound_page(uint64_t size);
        inline bool has_error() const noexcept;
        inline bool success() const noexcept;
        inline std::string error_string() const;
    private:
        Context get_current_context();
        void set_current_context(const Context &context);
        static void hookproc_interrupt(uc_engine *uc, uint32_t intno, void *user_data);
        static void hookproc_cmpxchg(uc_engine *uc, void *user_data);
        static void hookproc_execution(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
        static bool hookproc_read_memory(uc_engine *uc, uc_mem_type type,
            uint64_t address, int size, int64_t value, void *user_data);
    public:
        pid_t tid;  // emulate a remote thread(set its teb as gs)
    private:
        IBasicProcess &basic_process;
        IReadableProcess &readable_process;
        IExtraProcess &extra_process;
        erptr_t teb_address;
        uc_err err;
        uc_engine *uc;
        uc_hook hook_mem_unmapped;
        uc_hook hook_execution;
        uc_hook hook_interrupt;
        uc_hook hook_cmpxchg;
    private:
        size_t page_size;
        const rptr_t stack_size = 1024 * 1024 * 10;
        const rptr_t stack_base = 0x20000000000;
        const rptr_t stack_end = stack_base + stack_size;
    };
}