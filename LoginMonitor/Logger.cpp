#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/win_eventlog_sink.h"
#include <filesystem>
#include "Logger.h"

namespace fs = std::filesystem;

Logger::Logger(const std::string& serviceName)
{
	std::vector<spdlog::sink_ptr> sinks;

	sinks.push_back(std::make_shared<spdlog::sinks::win_eventlog_sink_st>(serviceName)); // log to event viewer
	fs::path logFilePath = fs::temp_directory_path() / "LogMonitor/LogMonitor.txt";
	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_st>(logFilePath.string())); // log to file

	m_Logger = std::make_shared<spdlog::logger>(serviceName, begin(sinks), end(sinks));
	spdlog::register_logger(m_Logger);
	spdlog::set_default_logger(m_Logger);
}
