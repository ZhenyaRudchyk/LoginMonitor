#include <Windows.h>
#include <list>
#include "Service.h"
#include "Logger.h"
#include "ProcessManager.h"
#include <processthreadsapi.h>


int _tmain(int argc, TCHAR* argv[])
{
    Logger log;
    Service service(L"LoginMonitor");
    return Service::Run(service);
}
