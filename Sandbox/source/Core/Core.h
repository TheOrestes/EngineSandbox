#pragma once

#ifdef BUILD_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#define LOG_CRITICAL(...)	Logger::getInstance().GetSandboxLogger()->critical(__VA_ARGS__);
#define LOG_ERROR(...)		Logger::getInstance().GetSandboxLogger()->error(__VA_ARGS__);
#define LOG_WARNING(...)	Logger::getInstance().GetSandboxLogger()->warn(__VA_ARGS__);
#define LOG_INFO(...)		Logger::getInstance().GetSandboxLogger()->info(__VA_ARGS__);
#define LOG_DEBUG(...)		Logger::getInstance().GetSandboxLogger()->debug(__VA_ARGS__);