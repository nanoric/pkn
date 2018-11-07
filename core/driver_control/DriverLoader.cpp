#include "DriverLoader.h"

#include <windows.h> 
#include <winsvc.h> 
#include <conio.h> 
#include <stdio.h>
#include <string>
#include <iostream>
#include <filesystem>

std::string error_to_string(DWORD last_error_code)
{
    //Get the error message, if any.
    DWORD errorMessageID = last_error_code;
    if (errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

bool DriverLoader::create_driver_service(const estr_t &eservice_name, const estr_t &driver_path)
{
    auto service_name = eservice_name.to_wstring();
    auto full_path = std::filesystem::absolute(driver_path.to<std::wstring>());
    bool retv = false;

    SC_HANDLE service_manager = nullptr;
    SC_HANDLE service = nullptr;

    service_manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (service_manager == nullptr)
    {
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        retv = false;
        goto _exit;
    }
    else
    {
        //printf("OpenSCManager() ok ! \n");
    }

    service = CreateServiceW(service_manager,
                             service_name.c_str(),
                             service_name.c_str(),
                             SERVICE_ALL_ACCESS,
                             SERVICE_KERNEL_DRIVER,
                             SERVICE_DEMAND_START,
                             SERVICE_ERROR_IGNORE,
                             full_path.wstring().c_str(),
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr
    );
    if (nullptr == service && GetLastError() == ERROR_SERVICE_EXISTS)
    {
        service = OpenServiceW(service_manager,
                               service_name.c_str(),
                               SERVICE_ALL_ACCESS);
        if (nullptr != service)
        {
            retv = ChangeServiceConfigW(service,
                                        SERVICE_KERNEL_DRIVER,
                                        SERVICE_DEMAND_START,
                                        SERVICE_ERROR_IGNORE,
                                        full_path.wstring().c_str(),
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr
            );
        }
    }
    else
    {
        retv = true;
    }
_exit:
    if (service)
    {
        CloseServiceHandle(service);
    }
    if (service_manager)
    {
        CloseServiceHandle(service_manager);
    }
    return retv;
}

bool DriverLoader::start_service(const estr_t &eservice_name)
{
    bool retv = false;
    SC_HANDLE service_manager = nullptr;
    SC_HANDLE service = nullptr;

    auto service_name = eservice_name.to_wstring();

    service_manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (service_manager == nullptr)
    {
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        goto _exit;
    }
    else
    {
        //printf("OpenSCManager() ok ! \n");
    }

    service = OpenServiceW(service_manager, service_name.c_str(), SERVICE_ALL_ACCESS);
    if (service == nullptr)
    {
        //printf("OpenService() Faild %d ! \n", GetLastError());
        goto _exit;
    }
    else
    {
        //printf("OpenService() ok ! \n");
    }

    if (service == nullptr)
    {
        auto lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING && lastError != ERROR_SERVICE_EXISTS)
        {
            //printf("CrateService() faild %d: %s\n", GetLastError(), error_to_string(GetLastError()).c_str());
            retv = false;
            goto _exit;
        }
        else
        {
            //printf("CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
        }

        service = OpenServiceW(service_manager, service_name.c_str(), SERVICE_ALL_ACCESS);
        if (service == nullptr)
        {
            //printf("OpenService() faild %d: %s\n", GetLastError(), error_to_string(GetLastError()).c_str());
            retv = false;
            goto _exit;
        }
        else
        {
            //printf("OpenService() ok ! \n");
        }
    }
    else
    {
        //printf("CrateService() ok ! \n");
    }

    retv = StartService(service, 0, nullptr);
    if (!retv)
    {
        auto lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING && lastError != ERROR_SERVICE_ALREADY_RUNNING)
        {
            //printf("StartService() faild %d: %s\n", GetLastError(), error_to_string(GetLastError()).c_str());
            retv = false;
            goto _exit;
        }
        else if (lastError == ERROR_IO_PENDING)
        {
            //printf("StartService() Faild ERROR_IO_PENDING ! \n");
            retv = false;
            goto _exit;
        }
        else
        {
            //printf("StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
            retv = true;
            goto _exit;
        }
    }
    retv = true;
_exit:
    if (service)
    {
        CloseServiceHandle(service);
    }
    if (service_manager)
    {
        CloseServiceHandle(service_manager);
    }
    return retv;
}

bool DriverLoader::stop_service(const estr_t &eservice_name)
{
    bool stop_succeed = false;
    SC_HANDLE service_manager = nullptr;
    SC_HANDLE service = nullptr;
    SERVICE_STATUS status;

    auto service_name = eservice_name.to_wstring();

    service_manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (service_manager == nullptr)
    {
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        goto _exit;
    }
    else
    {
        //printf("OpenSCManager() ok ! \n");
    }

    service = OpenServiceW(service_manager, service_name.c_str(), SERVICE_ALL_ACCESS);
    if (service == nullptr)
    {
        //printf("OpenService() Faild %d ! \n", GetLastError());
        goto _exit;
    }
    else
    {
        //printf("OpenService() ok ! \n");
    }

    if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
    {
        auto err = GetLastError();
        //printf("ControlService() faild %d: %s\n", err, error_to_string(err).c_str());
        //printf("*** A restart is needed!\n");
    }
    else
    {
        stop_succeed = true;
        //printf("ControlService() ok !\n");
    }
_exit:
    if (service)
    {
        CloseServiceHandle(service);
    }
    if (service_manager)
    {
        CloseServiceHandle(service_manager);
    }
    return stop_succeed;
}

bool DriverLoader::delete_service(const estr_t &eservice_name)
{
    bool delete_succeed = false;
    SC_HANDLE service_manager = nullptr;
    SC_HANDLE service = nullptr;

    auto service_name = eservice_name.to_wstring();

    service_manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (service_manager == nullptr)
    {
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        goto _exit;
    }
    else
    {
        //printf("OpenSCManager() ok ! \n");
    }

    service = OpenServiceW(service_manager, service_name.c_str(), SERVICE_ALL_ACCESS);
    if (service == nullptr)
    {
        //printf("OpenService() Faild %d ! \n", GetLastError());
        goto _exit;
    }
    else
    {
        //printf("OpenService() ok ! \n");
    }

    if (!DeleteService(service))
    {
        //printf("DeleteSrevice() Faild %d !\n", GetLastError());
    }
    else
    {
        delete_succeed = true;
        //printf("DelServerDeleteSrevice() ok !\n");
    }
_exit:
    if (service)
    {
        CloseServiceHandle(service);
    }
    if (service_manager)
    {
        CloseServiceHandle(service_manager);
    }
    return delete_succeed;
}

bool DriverLoader::enable_privilege(const char *name)
{
    TOKEN_PRIVILEGES privilege;
    HANDLE tokenHandle;

    privilege.PrivilegeCount = 1;
    privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!LookupPrivilegeValueA(NULL, name,
                               &privilege.Privileges[0].Luid))
        return false;

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES, &tokenHandle))
        return false;

    if (!AdjustTokenPrivileges(tokenHandle,
                               false,
                               &privilege,
                               sizeof(privilege),
                               nullptr,
                               nullptr))
    {
        CloseHandle(tokenHandle);
        return false;
    }

    CloseHandle(tokenHandle);
    return true;
}

bool DriverLoader::enable_load_driver_privilege()
{
    return enable_privilege(SE_LOAD_DRIVER_NAME);
}
