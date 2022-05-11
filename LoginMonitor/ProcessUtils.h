#pragma once

#include <string>
#include <intsafe.h>

namespace ProcessUtils
{
	void OpenProcessInSession(const DWORD sessionID, std::wstring& exePath);
	void OpenProcessForAllActiveUser(std::wstring& exePath);
};

