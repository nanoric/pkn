#include <Windows.h>

#include <stdexcept>
#include <string>
#include <ntddmou.h>

#include "Driver.h"

#include "../../../kernel/io_code.h"
#include "../../../kernel/names.h"

#define EncryptInputByXor() xor_memory((char *)&inp + 8, sizeof(inp) - 8, xor_key); inp.xor_val = this->xor_key
#define DecryptOutputByXor() xor_memory((char *)&oup, sizeof(oup), xor_key)

namespace pkn
{
    Driver::~Driver()
    {
        if (_handle && INVALID_HANDLE_VALUE != _handle)
            CloseHandle(_handle);
    }

    bool Driver::open(const wchar_t *device_name)
    {
        auto name = std::wstring(LR"(\??\)") + device_name;
        _handle = CreateFileW(name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        return (INVALID_HANDLE_VALUE != _handle);
    }

    void Driver::xor_memory(void *address, size_t size, uint64_t xor_key) noexcept
    {
        for (size_t i = 0; i + 8 <= size; i += 8)
        {
            *(uint64_t *)((char *)address + i) ^= xor_key;
        }
    }

    bool Driver::read_process_memory(pid_t pid, erptr_t remote_address, size_t size, void *buffer) const noexcept
    {
        ReadProcessMemoryInput inp = { xor_key, pid, remote_address, size, (uint64_t)buffer };
        //inp.xor_val = 0;
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_READ_PROCESS_MEMORY, &inp, sizeof(inp), nullptr, nullptr))
        {
            xor_memory(buffer, size, xor_key);
            return true;
        }
        return false;
    }

    bool Driver::write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const noexcept
    {
        WriteProcessMemoryInput inp = { xor_key, pid, remote_address, size, (uint64_t)data };
        EncryptInputByXor();
        // !!!!!!note: buffer for write process memory is not xor protected!!!!!
        return ioctl(IOCTL_PLAYERKNOWNS_WRITE_PROCESS_MEMORY, &inp, sizeof(inp), nullptr, nullptr);
    }

    bool Driver::force_write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const noexcept
    {
        uint64_t physical_address;
        if (_get_physical_memory_address(pid, remote_address, &physical_address))
            return _write_physical_memory(physical_address, size, data);
        return false;
    }

    bool Driver::virtual_query(pid_t pid, erptr_t remote_address, MEMORY_BASIC_INFORMATION *mbi) const noexcept
    {
        QueryVirtualMemoryInput inp = { xor_key, pid, remote_address };
        QueryVirtualMemoryOutput oup;
        uint32_t out_size = sizeof(oup);
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_QUERY_VIRTUAL_MEMORY, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            *mbi = oup.mbi;
            return true;
        }
        return false;
    }

    bool Driver::get_mapped_file(pid_t pid, uint64_t address, estr_t *mapped_file) const noexcept
    {
        GetMappedFileInput inp = { xor_key, pid, address };
        GetMappedFileOutput oup = { 0 };
        uint32_t out_size = sizeof(oup);
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_MAPPED_FILE, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            estr_t retv;
            retv.reserve(512);
            auto i = std::back_inserter(retv);
            for (auto p = oup.image_path; *p; p++)
            {
                *i++ = *p;
            }
            *mapped_file = retv;
            return true;
        }
        return false;
    }

    bool Driver::get_process_base(pid_t pid, erptr_t *base) const noexcept
    {
        GetProcessBaseInput inp = { xor_key, pid };

        GetProcessBaseOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_PROCESS_BASE, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            *base = oup.base;
            return true;
        }
        return false;
    }

    bool Driver::get_process_times(pid_t pid, uint64_t * pcreation_time, uint64_t * pexit_time, uint64_t * pkernel_time, uint64_t * puser_time) const noexcept
    {
        GetProcessTimesInput inp = { xor_key, pid };
        GetProcessTimesOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_PROCESS_TIMES, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if (pcreation_time) *pcreation_time = oup.creation_time;
            if (pexit_time) *pexit_time = oup.exit_time;
            if (pkernel_time) *pkernel_time = oup.kernel_time;
            if (puser_time) *pcreation_time = oup.user_time;
            return true;
        }
        return false;
    }

    bool Driver::get_process_name(pid_t pid, estr_t *name) const noexcept
    {
        GetProcessNameInput inp = { xor_key, pid };
        GetProcessNameOutput oup = { 0 };
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_PROCESS_NAME, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            estr_t retv;
            retv.reserve(512);
            auto i = std::back_inserter(retv);
            for (auto p = oup.process_name; *p; p++)
            {
                *i++ = *p;
            }
            *name = retv;
            return true;
        }
        return false;
    }

    erptr_t Driver::get_peb_address() const noexcept
    {
        return 0;
    }

    bool Driver::get_process_exit_status(pid_t pid, NTSTATUS *status)const noexcept
    {
        TestProcessInput inp = { xor_key, pid };

        TestProcessOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_PROCESS_EXIT_STATUS, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if(status) *status = oup.status;
            return true;
        }
        return false;
    }

    bool Driver::wait_for_process(pid_t pid, uint64_t timeout_nanosec, NTSTATUS *status) const noexcept
    {
        WaitProcessInput inp = { xor_key, pid, timeout_nanosec };

        WaitProcessOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_WAIT_FOR_PROCESS, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if(status) *status = oup.status;
            return true;
        }
        return false;
    }

    bool Driver::wait_for_thread(pid_t tid, uint64_t timeout_nanosec, NTSTATUS *status) const noexcept
    {
        WaitThreadInput inp = { xor_key, tid, timeout_nanosec };

        WaitThreadOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_WAIT_FOR_THREAD, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if(status) *status = oup.status;
            return true;
        }
        return false;
    }

    bool Driver::_get_physical_memory_address(pid_t pid, erptr_t remote_address, uint64_t *pphysical_address) const noexcept
    {
        return false;
        //throw kernel_not_implemented_error();
        //uint32_t outputSize = sizeof(*pphysical_address);
        //return ioctl(IOCTL_PLAYERKNOWNS_GET_PHISICAL_ADDRESS, &in, sizeof(in), pphysical_address, &outputSize);
    }

    bool Driver::_write_physical_memory(erptr_t remote_address, size_t size, const void *data) const noexcept
    {
        return false;
        //throw kernel_not_implemented_error();
        //struct input
        //{
        //    uint64_t startaddress;
        //    uint64_t bytestowrite;
        //};
        //auto buffer_size = size + sizeof(struct input);

        //char *buffer = new char[buffer_size];
        //auto in = (struct input *)buffer;
        //in->startaddress = remote_address;
        //in->bytestowrite = size;
        //memcpy((void *)((uintptr_t)in + sizeof(struct input)), data, size);

        //return ioctl(IOCTL_PLAYERKNOWNS_WRITE_PHISICAL_MEMORY, buffer, (decltype(in->bytestowrite))buffer_size);
    }


    bool Driver::create_user_thread(pid_t pid,
        IN PSECURITY_DESCRIPTOR psd OPTIONAL,
        IN bool CreateSuspended,
        IN uint64_t MaximumStackSize OPTIONAL,
        IN uint64_t CommittedStackSize OPTIONAL,
        IN uint64_t StartAddress,
        IN uint64_t Parameter OPTIONAL,
        OUT pid_t *out_pid OPTIONAL,
        OUT pid_t *tid OPTIONAL
    ) const noexcept
    {
        CreateUserThreadInput inp = { xor_key,
            pid,
            {},
            psd == nullptr ? false : true,
            CreateSuspended,
            MaximumStackSize,
            CommittedStackSize,
            StartAddress,
            Parameter
        };
        if (psd != nullptr)
        {
            memcpy(&inp.sd, psd, sizeof(SECURITY_DESCRIPTOR));
        }

        CreateUserThreadOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_CREATE_USER_THREAD, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            *out_pid = oup.pid;
            *tid = oup.tid;
            return true;
        }
        return false;
    }

    bool Driver::allocate_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t type, uint32_t protect, erptr_t *allocated_base, size_t *allocated_size)
    {
        AllocateVirtualMemoryInput inp = { xor_key,
            pid,
            (UINT64)address,
            size,
            type,
            protect
        };

        AllocateVirtualMemoryOutput oup;
        uint32_t out_size = sizeof(oup);
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_ALLOCATE_VIRTUAL_MEMORY, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if (allocated_base) *allocated_base = oup.address;
            if (allocated_size) *allocated_size = oup.size;
            return true;
        }
        return false;
    }


    bool Driver::free_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t type, erptr_t *freed_base /*= nullptr*/, size_t *freed_size /*= nullptr*/)
    {
        FreeVirtualMemoryInput inp = { xor_key,
            pid,
            (UINT64)address,
            size,
            type
        };
        FreeVirtualMemoryOutput oup;
        uint32_t out_size = sizeof(oup);
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_FREE_VIRTUAL_MEMORY, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if (freed_base) *freed_base = oup.address;
            if (freed_size) *freed_size = oup.size;
            return true;
        }
        return false;
    }

    bool Driver::protect_virtual_memory(pid_t pid, erptr_t address, size_t size, uint32_t protect, erptr_t *protected_base /*= nullptr*/, size_t *protected_size /*= nullptr*/, uint32_t *old_protect /*= nullptr*/)
    {
        ProtectVirtualMemoryInput inp = { xor_key,
            pid,
            (UINT64)address,
            size,
            protect
        };
        ProtectVirtualMemoryOutput oup;
        uint32_t out_size = sizeof(oup);
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_PROTECT_VIRTUAL_MEMORY, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            if (protected_base) *protected_base = oup.address;
            if (protected_size) *protected_size = oup.size;
            if (old_protect) *old_protect = oup.old_protect;
            return true;
        }
        return false;
    }

    bool Driver::get_mouse_pos(int *x, int *y) const noexcept
    {
        GetMousePosOutput oup;
        uint32_t out_size = sizeof(oup);

        if (ioctl(IOCTL_PLAYERKNOWNS_GET_MOUSE_POS, nullptr, 0, &oup, &out_size))
        {
            *x = oup.x;
            *y = oup.y;
            return true;
        }
        return false;
    }

    bool Driver::protect_process(pid_t pid) const noexcept
    {
        ProtectProcessInput inp{ xor_key, pid };
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_PROTECT_PROCESS, &inp, sizeof(inp), nullptr, nullptr))
            return true;
        return false;
    }

    bool Driver::unprotect_process() const noexcept
    {
        return ioctl(IOCTL_PLAYERKNOWNS_UNPROTECT_PROCESS, nullptr, 0, nullptr, nullptr);
    }

    //
    //void Driver::set_mouse_pos(int x, int y)
    //{
    //    SynthesizeMouseData data = { 0 };
    //    data.LastX = x;
    //    data.LastY = y;
    //    data.Flags = 0;
    //    return synthesize_mouse(&data, 1);
    //}
    //
    //void Driver::synthesize_mouse(SynthesizeMouseData *datas, size_t count)
    //{
    //    static_assert(sizeof(SynthesizeMouseData) == sizeof(SynthesizeMouseInputData));
    //
    //    uint8_t buffer[0x1000];
    //    bool need_free = false;
    //    const size_t data_size = sizeof(SynthesizeMouseData) * count;
    //    const size_t input_size = sizeof(SynthesizeMouseInputHead) + data_size;
    //    SynthesizeMouseInputHead *pinput;
    //    if (input_size <= sizeof(buffer))
    //    {
    //        pinput = (SynthesizeMouseInputHead *)buffer;
    //    }
    //    else
    //    {
    //        pinput = (SynthesizeMouseInputHead *)new char[input_size];
    //        need_free = true;
    //    }
    //    pinput->count = count;
    //    pinput->fill = 0;
    //    memcpy(pinput + 1, datas, data_size);
    //
    //    if (ioctl(IOCTL_PLAYERKNOWNS_SYNTHESIZE_MOUSE, pinput, (uint32_t)input_size, nullptr, nullptr))
    //    {
    //        if (need_free)
    //            delete pinput;
    //        return;
    //    }
    //
    //    if (need_free)
    //        delete pinput;
    //    throw kernel_synthesize_mouse_error();
    //}

    bool Driver::ioctl(uint32_t code, void *input, uint32_t input_size, void *output, uint32_t *poutput_size) const noexcept
    {
        //__try
        //{
            return DeviceIoControl(_handle, code, input, input_size, output, poutput_size != nullptr ? *poutput_size : 0, (LPDWORD)poutput_size, nullptr);
        //}
        //__except (1)
        //{
            //return false;
        //}
    }

    PknDriver::PknDriver()
    {
        this->open(PlayerKnownsDriverName);
    }
}
