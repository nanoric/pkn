#include "DriverLoader.h"

#include <windows.h> 
#include <winsvc.h> 
#include <conio.h> 
#include <stdio.h>
#include <string>
#include <iostream>

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


//装载NT驱动程序
BOOL LoadNTDriver(const char * driver_name, const char * driver_path)
{
    char driver_full_path[256];
    //得到完整的驱动路径
    GetFullPathNameA(driver_path, 256, driver_full_path, NULL);

    BOOL bRet = FALSE;

    SC_HANDLE service_manager = NULL;//SCM管理器的句柄
    SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄

                                 //打开服务控制管理器
    service_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (service_manager == NULL)
    {
        //OpenSCManager失败
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else
    {
        ////OpenSCManager成功
        //printf("OpenSCManager() ok ! \n");
    }

    //创建驱动所对应的服务
    hServiceDDK = CreateServiceA(service_manager,
        driver_name, //驱动程序的在注册表中的名字 
        driver_name, // 注册表驱动程序的 DisplayName 值 
        SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限 
        SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序 
        SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值 
        SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值 
        driver_full_path, // 注册表驱动程序的 ImagePath 值 
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    DWORD dwRtn;
    //判断服务是否失败
    if (hServiceDDK == NULL)
    {
        dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
        {
            //由于其他原因创建服务失败
            auto err = GetLastError();
            //printf("CrateService() faild %d: %s\n", err, error_to_string(err).c_str());
            bRet = FALSE;
            goto BeforeLeave;
        }
        else
        {
            //服务创建失败，是由于服务已经创立过
            //printf("CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
        }

        // 驱动程序已经加载，只需要打开 
        hServiceDDK = OpenServiceA(service_manager, driver_name, SERVICE_ALL_ACCESS);
        if (hServiceDDK == NULL)
        {
            //如果打开服务也失败，则意味错误
            dwRtn = GetLastError();
            //printf("OpenService() Faild %d ! \n", dwRtn);
            bRet = FALSE;
            goto BeforeLeave;
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

    //开启此项服务
    bRet = StartService(hServiceDDK, NULL, NULL);
    if (!bRet)
    {
        DWORD dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
        {
            auto err = GetLastError();
            //printf("StartService() faild %d: %s\n", err, error_to_string(err).c_str());
            bRet = FALSE;
            goto BeforeLeave;
        }
        else
        {
            if (dwRtn == ERROR_IO_PENDING)
            {
                //设备被挂住
                //printf("StartService() Faild ERROR_IO_PENDING ! \n");
                bRet = FALSE;
                goto BeforeLeave;
            }
            else
            {
                //服务已经开启
                //printf("StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
                bRet = TRUE;
                goto BeforeLeave;
            }
        }
    }
    bRet = TRUE;
    //离开前关闭句柄
BeforeLeave:
    if (hServiceDDK)
    {
        CloseServiceHandle(hServiceDDK);
    }
    if (service_manager)
    {
        CloseServiceHandle(service_manager);
    }
    return bRet;
}

//卸载驱动程序 
BOOL UnloadNTDriver(char * service_name)
{
    BOOL bRet = FALSE;
    SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
    SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄
    SERVICE_STATUS SvrSta;
    //打开SCM管理器
    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hServiceMgr == NULL)
    {
        //带开SCM管理器失败
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else
    {
        //带开SCM管理器失败成功
        printf("OpenSCManager() ok ! \n");
    }
    //打开驱动所对应的服务
    hServiceDDK = OpenServiceA(hServiceMgr, service_name, SERVICE_ALL_ACCESS);

    if (hServiceDDK == NULL)
    {
        //打开驱动所对应的服务失败
        //printf("OpenService() Faild %d ! \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else
    {
        //printf("OpenService() ok ! \n");
    }
    //停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
    if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
    {
        auto err = GetLastError();
        //printf("ControlService() faild %d: %s\n", err, error_to_string(err).c_str());
        //printf("*** A restart is needed!\n");
    }
    else
    {
        //打开驱动所对应的失败
        //printf("ControlService() ok !\n");
    }
    //动态卸载驱动程序。 
    if (!DeleteService(hServiceDDK))
    {
        //卸载失败
        //printf("DeleteSrevice() Faild %d !\n", GetLastError());
    }
    else
    {
        //卸载成功
        //printf("DelServerDeleteSrevice() ok !\n");
    }
    bRet = TRUE;
BeforeLeave:
    //离开前关闭打开的句柄
    if (hServiceDDK)
    {
        CloseServiceHandle(hServiceDDK);
    }
    if (hServiceMgr)
    {
        CloseServiceHandle(hServiceMgr);
    }
    return bRet;
}

void TestDriver()
{
    //测试驱动程序 
    const char *filename = R"(\??\HelloDDK)";
    printf("Testing Driver by CreateFile(%s)\n", filename);
    HANDLE hDevice = CreateFileA(filename,
        GENERIC_WRITE | GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        printf("Create Device ok ! \n");
    }
    else
    {
        auto err = GetLastError();
        printf("Create Device faild %d: %s\n", err, error_to_string(err).c_str());
    }
    CloseHandle(hDevice);
}
//
//int main(int argc, char* argv[])
//{
//    auto driver_name = argv[1];
//    auto driver_path = argv[2];
//    //加载驱动
//    BOOL bRet = LoadNTDriver(driver_name, driver_path);
//    if (!bRet)
//    {
//        printf("LoadNTDriver error\n");
//        return 0;
//    }
//    //加载成功
//
//    //printf("press any to create device!\n");
//    //getch();
//
//    TestDriver();
//
//    //这时候你可以通过注册表，或其他查看符号连接的软件验证。 
//    printf("press any to unload the driver!\n");
//    getch();
//
//    //卸载驱动
//    bRet = UnloadNTDriver(driver_name);
//    if (!bRet)
//    {
//        printf("UnloadNTDriver error\n");
//        return 0;
//    }
//    else
//    {
//        printf("UnloadNTDriver success\n");
//    }
//
//    return 0;
//}