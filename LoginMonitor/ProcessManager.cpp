#include "ProcessManager.h"
#include "ScopeGuard.h"
#include "Logger.h"
#include "SessionInfo.h"
#include <userenv.h>
#include <ntstatus.h>
#include <wtsapi32.h>


namespace ProcessUtils
{
	bool OpenProcessInSession(const DWORD sessionID, const std::string& exeName)
	{
		bool result = false;
		do
		{
			ScopeGuard<HANDLE, void(*)(HANDLE&)> hImpersonationToken([](HANDLE& data) { CloseHandle(data); });

			if (WTSQueryUserToken(sessionID, &hImpersonationToken.data) == NULL)
			{
				spdlog::error("Failed to get user token. Error code: {}", GetLastError());
				break;
			}

			Logger::logger()->info("WTSQueryUserToken successful");

			if (!hImpersonationToken.data)
			{
				spdlog::error("Failed to retrive user token. Error code: {}", GetLastError());
				break;
			}

			ScopeGuard<HANDLE, void(*)(HANDLE&)> hUserPrimaryToken([](HANDLE& data) { CloseHandle(data); });

			if (DuplicateTokenEx(hImpersonationToken.data,
				MAXIMUM_ALLOWED,
				NULL,
				SecurityDelegation,
				TokenPrimary,
				&hUserPrimaryToken.data) == NULL)
			{
				spdlog::error("Failed to duplicate token. Error code:{}", GetLastError());
				break;
			}

			if (!hUserPrimaryToken.data)
			{
				spdlog::error("Failed to retrive  hUserToken token. Error code: {}", GetLastError());
				break;
			}


			ScopeGuard<LPVOID, void(*)(LPVOID&)> environmentBlock([](LPVOID& data) { DestroyEnvironmentBlock(data); });

			if (!CreateEnvironmentBlock(&environmentBlock.data, hUserPrimaryToken.data, FALSE))
			{
				spdlog::error("Failed to create env block. Error code: {}", GetLastError());
				break;
			}

			DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;

			spdlog::info("Primary token was successfully created. Session ID: {}", sessionID);
			STARTUPINFOA si;
			ZeroMemory(&si, sizeof(si));

			si.lpDesktop = const_cast<char*>("winsta0\\default");
			si.cb = sizeof(STARTUPINFO);

			ScopeGuard<PROCESS_INFORMATION, void(*)(PROCESS_INFORMATION&)> pi([](PROCESS_INFORMATION& data) { CloseHandle(&data); });

			if (CreateProcessAsUserA(
				hUserPrimaryToken.data,
				exeName.c_str(),
				NULL,
				NULL,
				NULL,
				FALSE,
				dwCreationFlags,
				environmentBlock.data,
				NULL,
				&si,
				&pi.data) == NULL)
			{
				spdlog::error("Failed to run {}. Error code: {}", exeName, GetLastError());
				break;
			}

			spdlog::info("CreateProcessAsUserA successful");
			result = true;

		} while (false);

		return result;
	}



	bool OpenProcessForAllActiveUser(const std::string& exeName)
	{

		SessionsInfo sessionsManager;
		if (!sessionsManager.GetAllSessions())
		{
			return false;
		}

		auto Sessions = sessionsManager.AllSessions();

		for (DWORD i = 0; i < sessionsManager.AllSessionsCount(); ++i)
		{
			ScopeGuard<LPSTR, void(*)(LPSTR&)> pBuffer = { [](LPSTR& data) { WTSFreeMemory(data); } };
			DWORD pBytesReturned = 0;
			const DWORD sessionID = Sessions[i].SessionId;

			if (WTSQuerySessionInformationA(SessionsInfo::SERVER, sessionID, WTSConnectState, &pBuffer.data, &pBytesReturned) == NULL)
			{
				spdlog::error(" Failed to get session connection state. Error code: {}", GetLastError());
				continue;
			}

			WTS_CONNECTSTATE_CLASS sessionState = *reinterpret_cast<WTS_CONNECTSTATE_CLASS*>(pBuffer.data);

			if (sessionState != WTSActive)
			{
				continue;
			}

			ProcessUtils::OpenProcessInSession(sessionID, exeName);

		}

		return true;
	}
}