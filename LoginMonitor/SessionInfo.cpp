#include <Windows.h>
#include <WtsApi32.h>
#include "SessionInfo.h"
#include "Logger.h"

const HANDLE SessionsInfo::SERVER = WTS_CURRENT_SERVER_HANDLE;
const int SessionsInfo::VERSION = 1;

bool SessionsInfo::GetAllSessions()
{
	WTSFreeMemory(m_SessionInfoArray.data);
	if (!WTSEnumerateSessionsA(SERVER, NULL, VERSION, &m_SessionInfoArray.data, &m_SessionsCount))
	{
		spdlog::error("Failed to get all sessions. Error code: {}", GetLastError());
		return false;
	}
	return true;
}

const PWTS_SESSION_INFOA SessionsInfo::AllSessions() const noexcept
{
	return m_SessionInfoArray.data;
}

DWORD SessionsInfo::AllSessionsCount() const noexcept
{
	return m_SessionsCount;
}

