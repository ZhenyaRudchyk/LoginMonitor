#include "LoginMonitorService.h"
#include "Logger.h"


int _tmain(int argc, TCHAR* argv[])
{
    auto serviceName = L"LoginMonitor";
    Logger log("LoginMonitorLogger");
    LoginMonitorService service(serviceName, L"C:\\Windows\\System32\\notepad.exe");
    return LoginMonitorService::Run(service);
}
