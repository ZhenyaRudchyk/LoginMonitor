#pragma once

#include <memory>
#include "spdlog/spdlog.h"

class Logger
{
public:
	Logger(const std::string& serviceName);
	std::shared_ptr<spdlog::logger> getLogger();
private:
	std::shared_ptr<spdlog::logger> m_Logger;
};

