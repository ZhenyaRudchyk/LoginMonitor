#include "LoginMonitorService.h"
#include "Logger.h"
#include "ProcessUtils.h"

LoginMonitorService::LoginMonitorService(const std::wstring& serviceName, const std::wstring exePath): 
	BacisService(serviceName),
	m_ExePath(exePath)
{}

void LoginMonitorService::onStart()
{	
	ProcessUtils::OpenProcessForAllActiveUser(m_ExePath);
}

void LoginMonitorService::onStop()
{}

void LoginMonitorService::onPause()
{}

void LoginMonitorService::onContinue()
{}

std::string GetConnectionEventString(const int serviceState)
{
	switch (serviceState)
	{
	case WTS_REMOTE_CONNECT:
		return "WTS_REMOTE_CONNECT";
	case WTS_SESSION_LOGON:
		return "WTS_SESSION_LOGON";
	}
	return {};
}

void LoginMonitorService::onSessionChange(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	auto data = reinterpret_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
	if (!data)
	{
		spdlog::info("Failed to get WTSSESSION_NOTIFICATION data.");
		return;
	}

	const DWORD sessionID = data->dwSessionId;

	switch (dwEventType)
	{
	case WTS_REMOTE_CONNECT:
		spdlog::info("{} event received.", GetConnectionEventString(WTS_REMOTE_CONNECT));
		ProcessUtils::OpenProcessInSession(sessionID, m_ExePath);
		break;
	case WTS_SESSION_LOGON:
		spdlog::info("{} event received.", GetConnectionEventString(WTS_SESSION_LOGON));
		ProcessUtils::OpenProcessInSession(sessionID, m_ExePath);
		break;
	}
}

void LoginMonitorService::onShutdown()
{}