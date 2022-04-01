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

private:
	Logger();
	Logger(const Logger&);

	spdlog::logger* m_pLoggerSandbox;
};

