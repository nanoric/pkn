#include "UnicornEmulator.h"



UnicornEmulator::UnicornEmulator()
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

bool UnicornEmulator::init(KProcess *p)
{
    this->process = p;
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

    // hook cmpxchg
    //err = uc_hook_add(uc, &hook_cmpxchg, UC_HOOK_INSN, &UnicornEmulator::hookproc_cmpxchg, this, 1, 0, UC_X86_INS_CMPXCHG16B);

    return true;
}

Context UnicornEmulator::run(erptr_t start_addr, erptr_t end_addr, const EntryParameter &context)
{
    for (int i = 0; i < context.memories_number; i++)
    {
        const auto &memory = context.memories[i];
        err = uc_mem_map(uc, memory.base, upper_bound_page(memory.size), UC_PROT_READ | UC_PROT_WRITE | UC_PROT_EXEC);
        no_error_or_ret(err, get_current_context());

        err = uc_mem_write(uc, memory.base, memory.buffer, memory.size);
        no_error_or_ret(err, get_current_context());
    }

    {

        uint64_t _start_addr = start_addr;
        uc_reg_write(uc, UC_X86_REG_RIP, &_start_addr);
    }

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
        This->process->read_unsafe(aligned_base, piece_size, buffer);
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
