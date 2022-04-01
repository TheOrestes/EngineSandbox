
#include "sandboxPCH.h"
#include "Logger.h"

Logger::Logger()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	m_pLoggerSandbox = spdlog::default_logger_raw();
	m_pLoggerSandbox->set_level(spdlog::level::trace);
}
