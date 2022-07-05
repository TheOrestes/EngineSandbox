#pragma once

#include "sandboxPCH.h"
#include "Core/Core.h"

namespace Helper
{
	namespace App
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

		//-----------------------------------------------------------------------------------------------------------------------
		//--- Create Shader Module
		inline VkShaderModule CreateShaderModule(VkDevice device, const std::string& fileName)
		{
			// start reading at the end & in binary mode.
			// Advantage of reading file from the end is we can use read position to determine
			// size of the file & allocate buffer accordingly!
			std::ifstream file(fileName, std::ios::ate | std::ios::binary);

			if (!file.is_open())
				LOG_ERROR("Failed to open Shader file!");

			// get the file size & allocate buffer memory!
			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			// now seek back to the beginning of the file & read all bytes at once!
			file.seekg(0);
			file.read(buffer.data(), fileSize);

			// close the file!
			file.close();

			// Create Shader Module
			VkShaderModuleCreateInfo shaderModuleInfo;
			shaderModuleInfo.codeSize = buffer.size();
			shaderModuleInfo.flags = 0;
			shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
			shaderModuleInfo.pNext = nullptr;
			shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

			VkShaderModule shaderModule;
			std::string shaderModuleName = fileName;

			vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &shaderModule);

			return shaderModule;
		}
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