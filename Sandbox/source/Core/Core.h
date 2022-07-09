#pragma once

#include "Logger.h"

//---------------------------------------------------------------------------------------------------------------------
#define LOG_CRITICAL(...)	Logger::getInstance().GetSandboxLogger()->critical(__VA_ARGS__);
#define LOG_ERROR(...)		Logger::getInstance().GetSandboxLogger()->error(__VA_ARGS__);		
#define LOG_WARNING(...)	Logger::getInstance().GetSandboxLogger()->warn(__VA_ARGS__);
#define LOG_INFO(...)		Logger::getInstance().GetSandboxLogger()->info(__VA_ARGS__);
#define LOG_DEBUG(...)		Logger::getInstance().GetSandboxLogger()->debug(__VA_ARGS__);

//---------------------------------------------------------------------------------------------------------------------
const uint16_t gWindowWidht = 800;
const uint16_t gWindowHeight = 600;

//---------------------------------------------------------------------------------------------------------------------
template<typename T> void SAFE_DELETE(T*& a)
{
	delete a;
	a = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
#define CHECK(x)						\
{										\
	if (!x)								\
	{									\
		return false;					\
	}									\
}										\

//---------------------------------------------------------------------------------------------------------------------
#define VK_CHECK(x)																		\
{																						\
	if (x != VK_SUCCESS)																\
	{																					\
		LOG_ERROR("Detected Vulkan Error at {0}:{1}", __FILE__, __LINE__);				\
		return false;																	\
	}																					\
}																						\