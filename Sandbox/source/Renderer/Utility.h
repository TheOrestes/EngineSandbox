#pragma once

#include "sandboxPCH.h"
#include "Core/Core.h"

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

namespace Helper
{
	const std::string gShaderCompilerPath = "C:/VulkanSDK/1.3.204.1/Bin/glslc.exe";
	const uint16_t gWindowWidht = 960;
	const uint16_t gWindowHeight = 540;
	const uint16_t gMaxFramesDraws = 2;

	enum ePipeline
	{
		FORWARD,
		DEFERRED,
		RT
	};

	//--- list of validation layers...
	const std::vector<const char*> g_strValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef _DEBUG
	const bool g_bEnableValidationLayer = true;
#else
	const bool g_bEnableValidationLayer = false;
#endif // _DEBUG

	//--- list of device extensions
	const std::vector<const char*> g_strDeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//--- Default Swapchain attachments
	struct SwapchainAttachment
	{
		VkImage		image;
		VkImageView	imageView;
	};

	//-----------------------------------------------------------------------------------------------------------------------
	// VERTEX STRUCTURES

	struct VertexPC
	{
		VertexPC() : Position(glm::vec3(0)), Color(glm::vec3(1)) {}
		VertexPC(const glm::vec3& pos, const glm::vec3& col) : Position(pos), Color(col) {}

		glm::vec3 Position;
		glm::vec3 Color;
	};
}

//---------------------------------------------------------------------------------------------------------------------
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//---------------------------------------------------------------------------------------------------------------------
static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}