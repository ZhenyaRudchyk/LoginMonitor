#pragma once

#include <memory>
#include "spdlog/spdlog.h"

class Logger
{
public:
	Logger(const std::string& serviceName);
private:
	std::shared_ptr<spdlog::logger> m_Logger;
};

