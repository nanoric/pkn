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

    void Driver::xor_memory(void *address, size_t size, uint64_t xor_key)
    {
        for (size_t i = 0; i + 8 <= size; i += 8)
        {
            *(uint64_t *)((char *)address + i) ^= xor_key;
        }
    }

    void Driver::read_process_memory(pid_t pid, erptr_t remote_address, size_t size, void *buffer) const
    {
        ReadProcessMemoryInput inp = { xor_key, pid, remote_address, size, (uint64_t)buffer };
        //inp.xor_val = 0;
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_READ_PROCESS_MEMORY, &inp, sizeof(inp), nullptr, nullptr))
        {
            xor_memory(buffer, size, xor_key);
            return;
        }
        throw kernel_read_memory_error();
    }

    void Driver::write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const
    {
        WriteProcessMemoryInput inp = { xor_key, pid, remote_address, size, (uint64_t)data };
        EncryptInputByXor();
        // !!!!!!note: buffer for write process memory is not xor protected!!!!!
        if (ioctl(IOCTL_PLAYERKNOWNS_WRITE_PROCESS_MEMORY, &inp, sizeof(inp), nullptr, nullptr))
        {
            return;
        }
        throw kernel_write_memory_error();
    }

    void Driver::force_write_process_memory(pid_t pid, erptr_t remote_address, size_t size, const void *data) const
    {
        uint64_t physical_address;
        if (_get_physical_memory_address(pid, remote_address, &physical_address))
            if (_write_physical_memory(physical_address, size, data))
                return;
        throw kernel_get_physical_address_error();
    }

    void Driver::virtual_query(pid_t pid, erptr_t remote_address, MEMORY_BASIC_INFORMATION *mbi) const
    {
        QueryVirtualMemoryInput inp = { xor_key, pid, remote_address };
        QueryVirtualMemoryOutput oup;
        uint32_t out_size = sizeof(oup);
        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_QUERY_VIRTUAL_MEMORY, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            *mbi = oup.mbi;
            return;
        }
        throw kernel_query_virtual_memory_error();
    }

    estr_t Driver::get_mapped_file(pid_t pid, uint64_t address)
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
            return retv;
        }
        throw kernel_get_mapped_file_error();
    }

    erptr_t Driver::get_process_base(pid_t pid)
    {
        GetProcessBaseInput inp = { xor_key, pid };

        GetProcessBaseOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_PROCESS_BASE, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            return oup.base;
        }
        throw kernel_get_process_base_error();
    }

    void Driver::get_process_times(pid_t pid, uint64_t * pcreation_time, uint64_t * pexit_time, uint64_t * pkernel_time, uint64_t * puser_time)
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
            return;
        }
        throw kernel_get_process_times_error();
    }

    estr_t Driver::get_process_name(pid_t pid)
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
            return retv;
        }
        throw kernel_get_process_name_error();
    }

    NTSTATUS Driver::get_process_exit_status(pid_t pid)const
    {
        TestProcessInput inp = { xor_key, pid };

        TestProcessOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_GET_PROCESS_EXIT_STATUS, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            return oup.status;
        }
        throw kernel_get_process_exit_status_error();
    }

    NTSTATUS Driver::wait_for_process(pid_t pid) const
    {
        WaitProcessInput inp = { xor_key, pid };

        WaitProcessOutput oup;
        uint32_t out_size = sizeof(oup);

        EncryptInputByXor();
        if (ioctl(IOCTL_PLAYERKNOWNS_WAIT_FOR_PROCESS, &inp, sizeof(inp), &oup, &out_size))
        {
            DecryptOutputByXor();
            return oup.status;
        }
        throw kernel_wait_process_error();
    }

    bool Driver::_get_physical_memory_address(pid_t pid, erptr_t remote_address, uint64_t *pphysical_address) const
    {
        throw kernel_not_implemented_error();
        //uint32_t outputSize = sizeof(*pphysical_address);
        //return ioctl(IOCTL_PLAYERKNOWNS_GET_PHISICAL_ADDRESS, &in, sizeof(in), pphysical_address, &outputSize);
    }

    bool Driver::_write_physical_memory(erptr_t remote_address, size_t size, const void *data) const
    {
        throw kernel_not_implemented_error();
        //struct input
        //{
        //    UINT64 startaddress;
        //    UINT64 bytestowrite;
        //};
        //auto buffer_size = size + sizeof(struct input);

        //char *buffer = new char[buffer_size];
        //auto in = (struct input *)buffer;
        //in->startaddress = remote_address;
        //in->bytestowrite = size;
        //memcpy((void *)((uintptr_t)in + sizeof(struct input)), data, size);

        //return ioctl(IOCTL_PLAYERKNOWNS_WRITE_PHISICAL_MEMORY, buffer, (decltype(in->bytestowrite))buffer_size);
    }


    void Driver::get_mouse_pos(int *x, int *y)
    {
        GetMousePosOutput oup;
        uint32_t out_size = sizeof(oup);

        if (ioctl(IOCTL_PLAYERKNOWNS_GET_MOUSE_POS, nullptr, 0, &oup, &out_size))
        {
            *x = oup.x;
            *y = oup.y;
            return;
        }
        throw kernel_get_mouse_pos_error();
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

    bool Driver::ioctl(uint32_t code, void *input, uint32_t input_size, void *output, uint32_t *poutput_size) const
    {
        __try
        {
            return DeviceIoControl(_handle, code, input, input_size, output, poutput_size != nullptr ? *poutput_size : 0, (LPDWORD)poutput_size, nullptr);
        }
        __except (1)
        {
            return false;
        }
    }

    PknDriver::PknDriver()
    {
        this->open(PlayerKnownsDriverName);
    }
}
