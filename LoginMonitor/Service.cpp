#include "Service.h"
#include "Logger.h"
#include <userenv.h>
#include <iostream>
#include <mutex>
#include <functional>
#include <libloaderapi.h>
#include "ProcessManager.h"
#include "ScopeGuard.h"

Service* Service::s_Service = nullptr;

Service::Service(const std::wstring& servicename) :
		m_ServiceName(servicename)
		
	{
	ZeroMemory(&m_ServiceStatus, sizeof(m_ServiceStatus));
	m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	// The service runs in its own process.
	m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	m_ServiceStatus.dwWin32ExitCode = NO_ERROR;
	m_ServiceStatus.dwServiceSpecificExitCode = 0;
	m_ServiceStatus.dwCheckPoint = 0;
	m_ServiceStatus.dwWaitHint = 0;

	}

	int Service::Run(Service& service)
	{
		s_Service = &service;
		SERVICE_TABLE_ENTRY ServiceTable[] =
		{
			{const_cast<LPWSTR>(s_Service->m_ServiceName.c_str()), (LPSERVICE_MAIN_FUNCTION)Service::ServiceMain},
			{NULL, NULL}
		};

		return StartServiceCtrlDispatcher(ServiceTable);
	}

	bool Service::Register()
	{
		m_StatusHandle = RegisterServiceCtrlHandlerEx(m_ServiceName.c_str(), ServiceCtrlHandler, NULL);

		if (m_StatusHandle == NULL)
		{
			spdlog::error(L"Failed to register {} service. Error code: {}", m_ServiceName, GetLastError());
			return false;
		}
		return true;
	}

	bool Service::Start()
	{

		if (!SetStatusToService(SERVICE_START_PENDING))
		{
			return false;
		}
		
		
		m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
											 SERVICE_ACCEPT_SESSIONCHANGE |
											 SERVICE_ACCEPT_SHUTDOWN |
											 SERVICE_ACCEPT_PAUSE_CONTINUE;

		if (!SetStatusToService(SERVICE_RUNNING))
		{
			return false;
		}

		ProcessUtils::OpenProcessForAllActiveUser("notepad.exe");

		spdlog::info(L"Service {} started.", m_ServiceName);

		return true;
	}

	bool Service::Shutdown(DWORD exitCode)
	{
		return Stop(exitCode);
	}

	bool Service::Stop(DWORD exitCode)
	{
		if (!SetStatusToService(SERVICE_STOP_PENDING))
		{
			return false;
		}

		if (!SetStatusToService(SERVICE_STOPPED))
		{
			return false;
		}

		spdlog::info(L"Service {} stoped.", m_ServiceName);
		return true;
	}

	VOID WINAPI Service::ServiceMain(DWORD argc, LPTSTR* argv)
	{
		do
		{
			// Register our service control handler with the SCM
			if (!s_Service->Register())
				break;

			if (!s_Service->Start())
				break;


		} while (false);
	}


	void Service::Continue()
	{
		SetStatusToService(SERVICE_CONTINUE_PENDING);
		SetStatusToService(SERVICE_RUNNING);
		spdlog::info(L"Service {} resumed.", m_ServiceName);
	}

	void Service::Pause()
	{
		SetStatusToService(SERVICE_PAUSE_PENDING);
		SetStatusToService(SERVICE_PAUSED);
		spdlog::info(L"Service {} paused.", m_ServiceName);
	}


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


	DWORD WINAPI Service::ServiceCtrlHandler(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
	{
		DWORD Result = ERROR_CALL_NOT_IMPLEMENTED;

		switch (CtrlCode)
		{
		case SERVICE_CONTROL_STOP:
		{
			s_Service->Stop(0);
			Result = NO_ERROR;
			break;
		}
		case SERVICE_CONTROL_PAUSE:
		{
			s_Service->Pause();
			Result = NO_ERROR;
			break;
		}

		case SERVICE_CONTROL_CONTINUE:
		{
			s_Service->Continue();
			Result = NO_ERROR;
			break;
		}

		case SERVICE_CONTROL_SESSIONCHANGE:
		{
			auto data = reinterpret_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
			if (!data)
			{
				Result = ERROR_INVALID_DATA;
				break;
			}

			DWORD sessionID = data->dwSessionId; // Now you have the actual session ID

			Result = NO_ERROR;

			switch (dwEventType)
			{
			case WTS_REMOTE_CONNECT:
			//case WTS_SESSION_UNLOCK:
				Logger::logger()->info("{} event received.", GetConnectionEventString(WTS_REMOTE_CONNECT));
				ProcessUtils::OpenProcessInSession(sessionID, "notepad.exe");
				break;
			case WTS_SESSION_LOGON:
				Logger::logger()->info("{} event received.", GetConnectionEventString(WTS_SESSION_LOGON));
				ProcessUtils::OpenProcessInSession(sessionID, "notepad.exe");
				break;
			}
			break;
		}
		case SERVICE_CONTROL_SHUTDOWN:
		{
			s_Service->Shutdown(0);
			Result = NO_ERROR;
			break;
		}
		case SERVICE_CONTROL_INTERROGATE:
		{
			Result = NO_ERROR;
			break;
		}

		}
		return Result;
	}

	std::string GetServiceStateString(const int serviceState)
	{
		switch (serviceState)
		{
		case SERVICE_START_PENDING:
			return "SERVICE_START_PENDING";
		case SERVICE_RUNNING:
			return "SERVICE_RUNNING";
		case SERVICE_STOP_PENDING:
			return "SERVICE_STOP_PENDING";
		case SERVICE_STOPPED:
			return "SERVICE_STOPPED";
		default:
			break;
		}
		return {};
	}

	bool  Service::SetStatusToService(DWORD dwCurrentState,
		DWORD dwWin32ExitCode,
		DWORD dwWaitHint)
	{
		static DWORD dwCheckPoint = 1;

		m_ServiceStatus.dwCurrentState = dwCurrentState;
		m_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
		m_ServiceStatus.dwWaitHint = dwWaitHint;

		m_ServiceStatus.dwCheckPoint = ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) ? 0 : dwCheckPoint++;

		// Report the status of the service to the SCM.

		if (::SetServiceStatus(m_StatusHandle, &m_ServiceStatus) == FALSE)
		{
			spdlog::error("Failed to set service status {}. Error code: {}", GetServiceStateString(dwCurrentState), GetLastError());
			return false;
		}
		return true;
	}

