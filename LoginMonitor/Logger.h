#pragma once

#include <memory>
#include "spdlog/spdlog.h"


class Logger
{
public:
	Logger();
	static std::shared_ptr<spdlog::logger>& logger();
private:
	static std::shared_ptr<spdlog::logger> m_Logger;
};

