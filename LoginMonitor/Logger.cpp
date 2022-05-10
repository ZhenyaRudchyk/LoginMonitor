#include "Logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/win_eventlog_sink.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::shared_ptr<spdlog::logger> Logger::m_Logger = nullptr;

Logger::Logger()
{
	std::vector<spdlog::sink_ptr> sinks;

	sinks.push_back(std::make_shared<spdlog::sinks::win_eventlog_sink_st>("LoginMonitor"));
	fs::path logFilePath = std::filesystem::temp_directory_path() / "LogMonitor.txt";
	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_st>(logFilePath.string()));

	//std::cout << logFilePath << std::endl;
	m_Logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
	spdlog::register_logger(m_Logger);
	spdlog::set_default_logger(m_Logger);
	//m_Logger->info(logFilePath.string());

}

std::shared_ptr<spdlog::logger>& Logger::logger()
{
	return m_Logger;
}