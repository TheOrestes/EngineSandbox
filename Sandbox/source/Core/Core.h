#pragma once

#include "Logger.h"

#define LOG_CRITICAL(...)	Logger::getInstance().GetSandboxLogger()->critical(__VA_ARGS__);
#define LOG_ERROR(...)		Logger::getInstance().GetSandboxLogger()->error(__VA_ARGS__);
#define LOG_WARNING(...)	Logger::getInstance().GetSandboxLogger()->warn(__VA_ARGS__);
#define LOG_INFO(...)		Logger::getInstance().GetSandboxLogger()->info(__VA_ARGS__);
#define LOG_DEBUG(...)		Logger::getInstance().GetSandboxLogger()->debug(__VA_ARGS__);

const uint16_t gWindowWidht = 960;
const uint16_t gWindowHeight = 540;

template<typename T> void SAFE_DELETE(T*& a)
{
	delete a;
	a = nullptr;
}