#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"

class Logger
{
public:
	static Logger& getInstance()
	{
		static Logger instance;
		return instance;
	}

	inline spdlog::logger* GetSandboxLogger()	{ return m_pLoggerSandbox;  }
	inline spdlog::logger* GetGameLogger()		{ return m_pLoggerGame; }

private:
	Logger();
	Logger(const Logger&);

	spdlog::logger* m_pLoggerSandbox;
	spdlog::logger* m_pLoggerGame;
};

