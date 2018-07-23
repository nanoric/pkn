#include <WinSock2.h>
#include <Windows.h>

#include "UnicornEmulator.h"

extern "C"
{
#include "unicorn/include/unicorn/unicorn.h"
}

#pragma pack(push, 1)
struct SegmentDescriptor {
    union {
        struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            unsigned short limit0;
            unsigned short base0;
            unsigned char base1;
            unsigned char type : 4;
            unsigned char no_system : 1;      /* S flag */
            unsigned char dpl : 2;
            unsigned char present : 1;     /* P flag */
            unsigned char limit1 : 4;
            unsigned char avail : 1;
            unsigned char is_64_code : 1;  /* L flag */
            unsigned char db : 1;          /* DB flag */
            unsigned char granularity : 1; /* G flag */
            unsigned char base2;
#else
            unsigned char base2;
            unsigned char granularity : 1; /* G flag */
            unsigned char db : 1;          /* DB flag */
            unsigned char is_64_code : 1;  /* L flag */
            unsigned char avail : 1;
            unsigned char limit1 : 4;
            unsigned char present : 1;     /* P flag */
            unsigned char dpl : 2;
            unsigned char no_system : 1;      /* S flag */
            unsigned char type : 4;
            unsigned char base1;
            unsigned short base0;
            unsigned short limit0;
#endif
        };
        uint64_t qword;
    };
    SegmentDescriptor()
    {
        this->qword = 0;
    }
    SegmentDescriptor(uint32_t base, uint32_t limit, bool is_code, bool is_64bit_code)
    {
        this->qword = 0;  //clear the descriptor

        this->avail = 1;
        this->base0 = base & 0xffff;
        this->base1 = (base >> 16) & 0xff;
        this->base2 = base >> 24;
        if (limit > 0xfffff) {
            //need Giant granularity
            limit >>= 12;
            this->granularity = 1;
        }
        this->limit0 = limit & 0xffff;
        this->limit1 = limit >> 16;

        //some sane defaults
        this->dpl = 3;
        this->present = 1;
        if (is_code && is_64bit_code)
        {
            this->is_64_code = 1;
            this->db = 0;
        }
        else
        {
            this->db = 1;   //32 bit
        }
        this->type = is_code ? 0xb : 3;
        this->no_system = 1;  //code or data
    }
};
#pragma pack(pop)


namespace pkn
{
    UnicornEmulator::UnicornEmulator(IBasicProcess &basic_process,
        IReadableProcess &readable_process,
        IExtraProcess &extra_process,
        pid_t tid)
        : basic_process(basic_process),
        readable_process(readable_process),
        extra_process(extra_process),
        tid(tid)
    {
        if (page_size == 0)
        {
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            page_size = si.dwPageSize;
        }
    }

    UnicornEmulator::~UnicornEmulator()
    {
        if (uc)
        {
            uc_close(uc);
            uc = nullptr;
        }
    }

    bool UnicornEmulator::init()
    {
        err = uc_open(UC_ARCH_X86, UC_MODE_64, &uc);
        //err = uc_open(UC_ARCH_X86, UC_MODE_32, &uc);

        // hook any read operation of running code
        err = uc_hook_add(uc, &hook_mem_unmapped, UC_HOOK_MEM_UNMAPPED, &UnicornEmulator::hookproc_read_memory, this, 1, 0);
        no_error_or_ret(err, false);

        // for test purpose : hook execution
        // hook any read operation of running code
        err = uc_hook_add(uc, &hook_execution, UC_HOOK_CODE, &UnicornEmulator::hookproc_execution, this, 1, 0);
        no_error_or_ret(err, false);

        // hook interrupts
        //err = uc_hook_add(uc, &hook_interrupt, UC_HOOK_INTR, &UnicornEmulator::hookproc_interrupt, this, 1, 0);
        //no_error_or_ret(err, false);

        if (!set_thread_id(tid))
            return false;

        return true;
    }

    bool UnicornEmulator::set_thread_id(pid_t tid)
    {
        this->tid = tid;
        if (tid == 0)
            return true;

        teb_address = extra_process.get_teb_address(tid);
        return teb_address != rnullptr;
    }

    Context UnicornEmulator::run(erptr_t start_addr, erptr_t end_addr, const EntryParameter &context)
    {
        // map memories
        for (int i = 0; i < context.memories_number; i++)
        {
            const auto &memory = context.memories[i];
            err = uc_mem_map(uc, memory.base, upper_bound_page(memory.size), UC_PROT_READ | UC_PROT_WRITE | UC_PROT_EXEC);
            no_error_or_ret(err, get_current_context());

            err = uc_mem_write(uc, memory.base, memory.buffer, memory.size);
            no_error_or_ret(err, get_current_context());
        }

        // write rip and set gs(teb)
        {
            // rip
            uint64_t _start_addr = start_addr;
            uc_reg_write(uc, UC_X86_REG_RIP, &_start_addr);
        }

        // if thread emulation is enable(tid is not null), 
        // initialize GDT and set GS
        // set GS , pointing to TEB
        if(tid)
        {
            // initialize gdt: null gdt and an teb segment for gs
            SegmentDescriptor gdt[2] = {
                SegmentDescriptor(),                            // null gdt
                SegmentDescriptor(0, 0xfffff000, false, false)  // teb for gs
            };

            // map them into linear memory
            constexpr uint64_t gdt_address = 0x100000;
            constexpr uint64_t gdt_size = sizeof(gdt);

            err = uc_mem_map(uc, gdt_address, upper_bound_page(gdt_size), UC_PROT_READ);
            no_error_or_ret(err, get_current_context());

            err = uc_mem_write(uc, gdt_address, gdt, gdt_size);
            no_error_or_ret(err, get_current_context());

            // gdtr
            uc_x86_mmr gdtr;
            gdtr.base = gdt_address;
            gdtr.limit = gdt_size - 1;
            uc_reg_write(uc, UC_X86_REG_GDTR, &gdtr);

            // set gs: selector : 1, ring3
            uint64_t gs = 1 << 3 | 3;
            uc_reg_write(uc, UC_X86_REG_GS, &gs);
        }

        // write some common registers (custom context)
        set_current_context(context.context);
        no_error_or_ret(err, get_current_context());

        // start
        try
        {
            err = uc_emu_start(uc, start_addr, end_addr, 0, 0);
        }
        catch (const std::exception&)
        {
        }

        // return context for user to get wanted result
        auto c = get_current_context();
        return c;
    }

    bool UnicornEmulator::hookproc_read_memory(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data)
    {
        UnicornEmulator *This = (UnicornEmulator *)user_data;
        auto page_size = This->page_size;
        size_t piece_size = page_size * 16;
        rptr_t aligned_base = address / piece_size * piece_size;
        std::vector<uint8_t> vbuffer(piece_size);
        auto *buffer = &vbuffer[0];
        auto *pcode = buffer + address - aligned_base;

        try
        {
            This->readable_process.read_unsafe(aligned_base, piece_size, buffer);
        }
        catch (const std::exception&)
        {
            return false;
        }

        This->err = uc_mem_map(uc, aligned_base, piece_size, UC_PROT_ALL);
        if (This->err != UC_ERR_OK)
        {
            //while (piece_size > page_size)
            //{
            //    piece_size -= page_size;
            //    This->err = uc_mem_map(uc, aligned_base, piece_size, UC_PROT_ALL);
            //    if (This->err != UC_ERR_OK)
            //        return true;
            //}

            return false;
        }
        //throw std::runtime_error("unable to map memory");
        This->err = uc_mem_write(uc, aligned_base, buffer, piece_size);

        if (This->err != UC_ERR_OK)
            //throw std::runtime_error("unable to write map memory");
            return false;
        return true;
    }

    Context UnicornEmulator::get_current_context()
    {
        Context context;
        constexpr int regs[] = {
            UC_X86_REG_RAX,
            UC_X86_REG_RBX,
            UC_X86_REG_RCX,
            UC_X86_REG_RDX,
            UC_X86_REG_R8,
            UC_X86_REG_R9,
            UC_X86_REG_R10,
            UC_X86_REG_R11,
            UC_X86_REG_R12,
            UC_X86_REG_RSP,
            UC_X86_REG_RBP,
            UC_X86_REG_RSI,
            UC_X86_REG_RDI,
            UC_X86_REG_RIP,
            UC_X86_REG_XMM0,
            UC_X86_REG_XMM1,
            UC_X86_REG_XMM2,
            UC_X86_REG_XMM3,
            UC_X86_REG_EFLAGS,
        };
        void *ptrs[] = {
            (void *)&context.rax,
            (void *)&context.rbx,
            (void *)&context.rcx,
            (void *)&context.rdx,
            (void *)&context.r8,
            (void *)&context.r9,
            (void *)&context.r10,
            (void *)&context.r11,
            (void *)&context.r12,
            (void *)&context.rsp,
            (void *)&context.rbp,
            (void *)&context.rsi,
            (void *)&context.rdi,
            (void *)&context.rip,
            (void *)&context.xmm0,
            (void *)&context.xmm1,
            (void *)&context.xmm2,
            (void *)&context.xmm3,
            (void *)&context.eflags,
        };
        err = uc_reg_read_batch(uc, (int *)regs, ptrs, sizeof(regs) / sizeof(*regs));
        return context;
    }

    void UnicornEmulator::set_current_context(const Context &context)
    {
        constexpr int regs[] = {
            UC_X86_REG_RAX,
            UC_X86_REG_RBX,
            UC_X86_REG_RCX,
            UC_X86_REG_RDX,
            UC_X86_REG_R8,
            UC_X86_REG_R9,
            UC_X86_REG_R10,
            UC_X86_REG_R11,
            UC_X86_REG_R12,
            UC_X86_REG_RSP,
            UC_X86_REG_RBP,
            UC_X86_REG_RSI,
            UC_X86_REG_RDI,
            UC_X86_REG_RIP,
            UC_X86_REG_XMM0,
            UC_X86_REG_XMM1,
            UC_X86_REG_XMM2,
            UC_X86_REG_XMM3,
            UC_X86_REG_EFLAGS,
        };
        void *ptrs[] = {
            (void *)&context.rax,
            (void *)&context.rbx,
            (void *)&context.rcx,
            (void *)&context.rdx,
            (void *)&context.r8,
            (void *)&context.r9,
            (void *)&context.r10,
            (void *)&context.r11,
            (void *)&context.r12,
            (void *)&context.rsp,
            (void *)&context.rbp,
            (void *)&context.rsi,
            (void *)&context.rdi,
            (void *)&context.rip,
            (void *)&context.xmm0,
            (void *)&context.xmm1,
            (void *)&context.xmm2,
            (void *)&context.xmm3,
            (void *)&context.eflags,
        };
        err = uc_reg_write_batch(uc, (int *)regs, ptrs, sizeof(regs) / sizeof(*regs));
    }

    uint64_t UnicornEmulator::upper_bound_page(uint64_t size)
    {
        return (size + page_size - 1) / page_size * page_size;
    }

    std::string UnicornEmulator::error_string() const
    {
        return uc_strerror(err);
    }

    void UnicornEmulator::hookproc_interrupt(uc_engine *uc, uint32_t intno, void *user_data)
    {
        int a = 1;
    }

    void UnicornEmulator::hookproc_cmpxchg(uc_engine *uc, void *user_data)
    {
        rptr_t rip;
        uc_reg_read(uc, UC_X86_REG_RIP, &rip);
        uint64_t address = rip;

        UnicornEmulator *This = (UnicornEmulator *)user_data;
        uint8_t ins[100];
        static auto *pins = ins;
        pins = ins;
        uc_mem_read(uc, address, ins, 100);
        address;
        user_data;
        rptr_t rax;
        rptr_t rdx;
        rptr_t rbx;
        rptr_t rcx;
        rptr_t r8;
        uint128_t xmm0;
        uint128_t xmm1;
        uint128_t pr8;

        uc_reg_read(uc, UC_X86_REG_RAX, &rax);
        uc_reg_read(uc, UC_X86_REG_RDX, &rdx);
        uc_reg_read(uc, UC_X86_REG_RBX, &rbx);
        uc_reg_read(uc, UC_X86_REG_RCX, &rcx);
        uc_reg_read(uc, UC_X86_REG_R8, &r8);
        uc_reg_read(uc, UC_X86_REG_XMM1, &xmm0);
        uc_reg_read(uc, UC_X86_REG_XMM2, &xmm1);
        uc_mem_read(uc, r8, &pr8, 16);

        printf_s("hookproc_cmpxchg at %llx\n", address);
        int a = 1;
    }

    void UnicornEmulator::hookproc_execution(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
    {
        UnicornEmulator *This = (UnicornEmulator *)user_data;
        uint8_t ins[100];
        static auto *pins = ins;
        pins = ins;
        uc_mem_read(uc, address, ins, 100);
        address;
        size;
        user_data;
        rptr_t rip;
        rptr_t rax;
        rptr_t rdx;
        rptr_t rbx;
        rptr_t rcx;
        rptr_t r8;
        uint128_t xmm0;
        uint128_t xmm1;
        uint128_t pr8;

        uc_reg_read(uc, UC_X86_REG_RIP, &rip);
        uc_reg_read(uc, UC_X86_REG_RAX, &rax);
        uc_reg_read(uc, UC_X86_REG_RDX, &rdx);
        uc_reg_read(uc, UC_X86_REG_RBX, &rbx);
        uc_reg_read(uc, UC_X86_REG_RCX, &rcx);
        uc_reg_read(uc, UC_X86_REG_R8, &r8);
        uc_reg_read(uc, UC_X86_REG_XMM1, &xmm0);
        uc_reg_read(uc, UC_X86_REG_XMM2, &xmm1);
        uc_mem_read(uc, r8, &pr8, 16);

        printf_s("running at %llx , %d\n", address, size);
        int a = 1;
    }

    bool UnicornEmulator::has_error() const noexcept
    {
        return err != UC_ERR_OK;
    }

    bool UnicornEmulator::success() const noexcept
    {
        return err == UC_ERR_OK;
    }

}

