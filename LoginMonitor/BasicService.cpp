#include <userenv.h>
#include <libloaderapi.h>
#include "ProcessUtils.h"
#include "ScopeGuard.h"
#include "BasicService.h"
#include "Logger.h"

BacisService* BacisService::s_Service = nullptr;

BacisService::BacisService(const std::wstring& servicename) :
	m_ServiceName(servicename)
	{
		ZeroMemory(&m_ServiceStatus, sizeof(m_ServiceStatus));
		m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

		// The service runs in its own process.
		m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

		m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
										SERVICE_ACCEPT_SESSIONCHANGE |
										SERVICE_ACCEPT_SHUTDOWN |
										SERVICE_ACCEPT_PAUSE_CONTINUE;

		m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		m_ServiceStatus.dwWin32ExitCode = NO_ERROR;
		m_ServiceStatus.dwServiceSpecificExitCode = 0;
		m_ServiceStatus.dwCheckPoint = 0;
		m_ServiceStatus.dwWaitHint = 0;
	}

	int BacisService::Run(BacisService& service)
	{
		s_Service = &service;
		SERVICE_TABLE_ENTRY ServiceTable[] =
		{
			{const_cast<LPWSTR>(s_Service->m_ServiceName.c_str()), (LPSERVICE_MAIN_FUNCTION)BacisService::ServiceMain},
			{NULL, NULL}
		};

		return StartServiceCtrlDispatcher(ServiceTable);
	}

	bool BacisService::Register()
	{
		m_StatusHandle = RegisterServiceCtrlHandlerEx(m_ServiceName.c_str(), ServiceCtrlHandler, NULL);

		if (m_StatusHandle == NULL)
		{
			spdlog::error(L"Failed to register {} service. Error code: {}", m_ServiceName, GetLastError());
			return false;
		}
		return true;
	}

	bool BacisService::Start()
	{

		if (!SetStatusToService(SERVICE_START_PENDING))
		{
			return false;
		}

		if (!SetStatusToService(SERVICE_RUNNING))
		{
			return false;
		}

		onStart();

		spdlog::info(L"Service {} started.", m_ServiceName);

		return true;
	}

	bool BacisService::Shutdown()
	{
		onShutdown();

		if (!SetStatusToService(SERVICE_STOPPED))
		{
			return false;
		}

		return true;
	}

	bool BacisService::Stop()
	{
		if (!SetStatusToService(SERVICE_STOP_PENDING))
		{
			return false;
		}

		onStop();

		if (!SetStatusToService(SERVICE_STOPPED))
		{
			return false;
		}

		spdlog::info(L"Service {} stoped.", m_ServiceName);
		return true;
	}

	VOID WINAPI BacisService::ServiceMain(DWORD argc, LPTSTR* argv)
	{
		do
		{
			if (!s_Service->Register())
				break;

			if (!s_Service->Start())
				break;


		} while (false);
	}


	void BacisService::Continue()
	{
		SetStatusToService(SERVICE_CONTINUE_PENDING);

		onContinue();

		SetStatusToService(SERVICE_RUNNING);
		spdlog::info(L"Service {} resumed.", m_ServiceName);
	}

	void BacisService::Pause()
	{
		SetStatusToService(SERVICE_PAUSE_PENDING);

		onPause();

		SetStatusToService(SERVICE_PAUSED);
		spdlog::info(L"Service {} paused.", m_ServiceName);
	}

	DWORD WINAPI BacisService::ServiceCtrlHandler(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
	{
		DWORD Result = ERROR_CALL_NOT_IMPLEMENTED;

		switch (CtrlCode)
		{
			case SERVICE_CONTROL_STOP:
			{
				s_Service->Stop();
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

				s_Service->onSessionChange(CtrlCode, dwEventType, lpEventData, lpContext);
				break;
			}
			case SERVICE_CONTROL_SHUTDOWN:
			{
				s_Service->Shutdown();
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

	bool  BacisService::SetStatusToService(DWORD dwCurrentState,
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

