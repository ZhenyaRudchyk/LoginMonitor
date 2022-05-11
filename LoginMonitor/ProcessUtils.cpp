#include <userenv.h>
#include "ProcessUtils.h"
#include "ScopeGuard.h"
#include "Logger.h"
#include "SessionInfo.h"

namespace ProcessUtils
{
	void OpenProcessInSession(const DWORD sessionID, std::wstring& exePath)
	{
		do
		{
			ScopeGuard<HANDLE, void(*)(HANDLE&)> hImpersonationToken([](HANDLE& data) { CloseHandle(data); });

			if (WTSQueryUserToken(sessionID, &hImpersonationToken.data) == NULL)
			{
				spdlog::error("Failed to get user token. Error code: {}", GetLastError());
				break;
			}

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

			STARTUPINFOW si;
			ZeroMemory(&si, sizeof(si));

			si.lpDesktop = LPWSTR(L"winsta0\\default");
			si.cb = sizeof(STARTUPINFO);

			ScopeGuard<PROCESS_INFORMATION, void(*)(PROCESS_INFORMATION&)> pi([](PROCESS_INFORMATION& data) { CloseHandle(&data); });

			if (CreateProcessAsUserW(
				hUserPrimaryToken.data,
				NULL,
				const_cast<wchar_t*>(exePath.c_str()),
				NULL,
				NULL,
				FALSE,
				dwCreationFlags,
				environmentBlock.data,
				NULL,
				&si,
				&pi.data) == NULL)
			{
				spdlog::error(L"Failed to run {}. Error code: {}", exePath, GetLastError());
				break;
			}

			spdlog::info(L"{} was successfully started in session = {}", exePath, sessionID);

		} while (false);
	}



	void OpenProcessForAllActiveUser(std::wstring& exePath)
	{
		SessionsInfo sessionsManager;
		if (!sessionsManager.GetAllSessions())
		{
			return;
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

			ProcessUtils::OpenProcessInSession(sessionID, exePath);
		}
	}
}