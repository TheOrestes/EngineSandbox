#pragma once

#include "sandboxPCH.h"
#include "Core/Core.h"

#include "Renderer/VulkanDevice.h"


namespace Helper
{
	namespace App
	{

	}

	namespace Vulkan
	{
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
	}
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