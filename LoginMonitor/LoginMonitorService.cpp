#include "LoginMonitorService.h"
#include "Logger.h"
#include "ProcessUtils.h"

LoginMonitorService::LoginMonitorService(const std::wstring& serviceName, const std::wstring exePath): 
	BacisService(serviceName),
	isPaused(false),
	m_ExePath(exePath)
{}

void LoginMonitorService::onStart()
{	
	ProcessUtils::OpenProcessForAllActiveUser(m_ExePath);
}

void LoginMonitorService::onStop()
{}

void LoginMonitorService::onPause()
{
	isPaused = true;
}

void LoginMonitorService::onContinue()
{
	isPaused = false;
}

void LoginMonitorService::onShutdown()
{
}

DWORD LoginMonitorService::onSessionChange(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	DWORD result = NO_ERROR;
	do
	{
		if (isPaused)
		{
			break;
		}

		auto data = static_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
		if (!data)
		{
			spdlog::info("Failed to get WTSSESSION_NOTIFICATION data.");
			result = ERROR_INVALID_DATA;
			break;
		}

		if (dwEventType == WTS_SESSION_LOGON)
		{
			spdlog::info("WTS_SESSION_LOGON event received. Session ID = {}", data->dwSessionId);
			ProcessUtils::OpenProcessInSession(data->dwSessionId, m_ExePath);
		}
	} while (false);

	return result;
}
