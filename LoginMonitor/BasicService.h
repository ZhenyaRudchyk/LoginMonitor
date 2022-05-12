#pragma once

#include <tchar.h>
#include <Windows.h>
#include <string>
#include <memory>
#include <utility>
#include "ServiceBase.h"
#include "ScopeGuard.h"

// Bacis class for Windows Service
class BacisService
{
public:
	BacisService(const std::wstring& servicename);
	virtual ~BacisService() = default;
	static int Run(BacisService& service);
	virtual void onStart() = 0;
	virtual void onStop() = 0;
	virtual void onPause() = 0;
	virtual void onContinue() = 0;
	virtual	DWORD onSessionChange(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) = 0;
	virtual void onShutdown() = 0;
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
	bool Stop();
	bool Shutdown();

	// The singleton service instance.
	static BacisService* s_Service;

	std::wstring m_ServiceName;
	SERVICE_STATUS m_ServiceStatus;
	SERVICE_STATUS_HANDLE m_StatusHandle;
};
