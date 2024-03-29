#include "work.hpp"
#include <windows.h>
#include <winsvc.h>
#include <iostream>
#include <string>

namespace service {

void start();
void service_main(DWORD argc, LPSTR *argv);
void service_control(DWORD ctrl_code);
BOOL ReportSvcStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode, DWORD dwWaitHint);

SERVICE_STATUS_HANDLE service_status_handle;
SERVICE_STATUS service_statu;
LPSTR service_name = "test_service";

void start() {
    SERVICE_TABLE_ENTRYA service_table[] = {
        {service_name, (LPSERVICE_MAIN_FUNCTIONA)service_main},
        {NULL, NULL}
    };
    WriteLog("enter main.");

    if(StartServiceCtrlDispatcher(service_table) ==0) {
        WriteLog("StartServiceCtrlDispatcher fail.");
        return;
    }
}

void service_main(DWORD argc, LPSTR *argv) {
    // 注册控制函数
    service_status_handle = RegisterServiceCtrlHandler(service_name, service_control);
    if(service_status_handle == 0) {
        WriteLog("RegisterServiceCtrlHandler fail.");
        return;
    }

    // 告诉SCM，当前服务正在启动中
    // dwWaitHint=3000ms是预估的时间. 
    // 如果超过这个时间，且dwCheckPoint 尚未递增或 dwCurrentState 未更改，则服务控制管理器或服务控制程序可以假定出错并且应停止服务
    if(ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000) == 0) {
        WriteLog("start fail");
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        WriteLog(std::to_string(GetLastError()).c_str());
        return;
    }

    // 准备工作

    // 做完准备工作后，告诉SCM服务已经启动
    if(ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0) ==0) {
        WriteLog("run fail");
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // 运行正在工作代码，通常是个无限循环
    WriteLog("work.run()");
    work::instance().run();
    return;
}

void service_control(DWORD ctrl_code) {
    switch(ctrl_code) {
        case SERVICE_CONTROL_SHUTDOWN: // 系统关闭
        case SERVICE_CONTROL_STOP: // 服务停止
            // call stop funciton
            ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
            WriteLog("work.stop()");
            work::instance().stop();
            ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 3000);
            break;
        default:
            break;
    }
}

BOOL ReportSvcStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode, DWORD dwWaitHint) {
    static DWORD dwCheckPoint = 1;
    // Fill in the SERVICE_STATUS structure.

    // 官方提供的示例中没有指定服务：https://learn.microsoft.com/zh-cn/windows/win32/services/sample-mc
    // 开始的时候，我没有下面这一行。我用sc.exe type=own指定，但是SetServiceStatus报错为ERROR_INVALID_DATA
    // 不知道是不是代码中必须要指定，否则会报错
    service_statu.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

    service_statu.dwCurrentState = dwCurrentState;
    service_statu.dwWin32ExitCode = dwWin32ExitCode;
    service_statu.dwWaitHint = dwWaitHint; // 挂起开始、停止、暂停或继续操作所需的估计时间

    if (dwCurrentState == SERVICE_START_PENDING) {
        service_statu.dwControlsAccepted = 0; // 正在启动，此时不接受控制码
    } else {
        service_statu.dwControlsAccepted = SERVICE_ACCEPT_STOP; // 服务可以停止。此控制代码允许服务接收 SERVICE_CONTROL_STOP 通知
    }

    // 服务定期递增的检查点值，以在长时间的启动、停止、暂停或继续操作期间报告其进度
    if ( (dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED) )
        service_statu.dwCheckPoint = 0;
    else service_statu.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    return SetServiceStatus(service_status_handle, &service::service_statu);
}
}; // namespace service 