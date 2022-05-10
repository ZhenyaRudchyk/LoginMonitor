#pragma once
#include <string>
#include <intsafe.h>

namespace ProcessUtils
{
	bool OpenProcessInSession(const DWORD sessionID, const std::string& exeName);
	bool OpenProcessForAllActiveUser(const std::string& exeName);
};

