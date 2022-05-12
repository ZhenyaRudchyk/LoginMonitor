#pragma once

#include "BasicService.h"

class LoginMonitorService : public BacisService
{
public:
	LoginMonitorService(const std::wstring& serviceName, const std::wstring exePath);

	void onStart() override;
	void onStop() override;
	void onPause() override;
	void onContinue() override;
	void onSessionChange(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) override;
	void onShutdown() override;
private:
	std::wstring m_ExePath;
};

