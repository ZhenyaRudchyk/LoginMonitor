#pragma once

#include <Windows.h>
#include <WtsApi32.h>
#include "ScopeGuard.h"

class SessionsInfo
{
public:
	const static HANDLE SERVER;
	bool GetAllSessions();
	const PWTS_SESSION_INFOA const AllSessions() const noexcept;
	DWORD AllSessionsCount() const noexcept;
private:
	DWORD m_SessionsCount{ 0 };
	ScopeGuard<PWTS_SESSION_INFOA, void(*)(PWTS_SESSION_INFOA&)> m_SessionInfoArray{ [](PWTS_SESSION_INFOA& data) { WTSFreeMemory(data); } };
};