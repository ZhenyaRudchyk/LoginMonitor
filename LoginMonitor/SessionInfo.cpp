#include <Windows.h>
#include <WtsApi32.h>
#include "SessionInfo.h"
#include "Logger.h"


const HANDLE SessionsInfo::SERVER = WTS_CURRENT_SERVER_HANDLE;

bool SessionsInfo::GetAllSessions()
{
	WTSFreeMemory(m_SessionInfoArray.data);
	if (!WTSEnumerateSessionsA(SERVER, 0, 1, &m_SessionInfoArray.data, &m_SessionsCount))
	{
		spdlog::error("Failed to get all sessions. Error code: {}", GetLastError());
		return false;
	}

	spdlog::info("All sessions count: {}\n", m_SessionsCount);
	return true;
}


const PWTS_SESSION_INFOA const SessionsInfo::AllSessions() const noexcept
{
	return m_SessionInfoArray.data;
}


DWORD SessionsInfo::AllSessionsCount() const noexcept
{
	return m_SessionsCount;
}

