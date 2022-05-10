#pragma once
#include <tchar.h>
#include <string>
#include <Windows.h>
#include <memory>
#include "ServiceBase.h"
#include <WtsApi32.h>
#include <utility>
#include <WTypesbase.h>
#include "ScopeGuard.h"


class Service
{
public:
	Service(const std::wstring& servicename);
	static int Run(Service& service);
private:

	static void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
	bool SetStatusToService(DWORD dwCurrentState,
		DWORD dwWin32ExitCode = NO_ERROR,
		DWORD dwWaitHint = 0);
	static DWORD WINAPI ServiceCtrlHandler(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

	bool Register();
	bool Start();
	void Pause();
	void Continue();
	bool Stop(DWORD exitCode = 0);
	bool Shutdown(DWORD exitCode = 0);

	// The singleton service instance.
	static Service* s_Service;

	std::wstring m_ServiceName;
	SERVICE_STATUS m_ServiceStatus;
	SERVICE_STATUS_HANDLE m_StatusHandle;
};
