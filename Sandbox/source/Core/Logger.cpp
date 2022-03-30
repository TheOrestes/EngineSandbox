
#include "Logger.h"

Logger::Logger()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	m_pLoggerSandbox = spdlog::default_logger_raw();
	m_pLoggerSandbox->set_level(spdlog::level::trace);

	m_pLoggerGame = spdlog::default_logger_raw();
	m_pLoggerGame->set_level(spdlog::level::trace);
}
