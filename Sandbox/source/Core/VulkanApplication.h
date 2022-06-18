#pragma once

#include "IApplication.h"
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "Core.h"

class VulkanRenderer;

class VulkanApplication : public IApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

	virtual bool				Initialize(void* pWindow) override;
	virtual void				Update(float dt) override;
	virtual void				Render() override;
	virtual void				Destroy() override;

private:
	VulkanApplication(const VulkanApplication&);
	VulkanApplication& operator=(const VulkanApplication&);

	bool						CreateInstance();
	bool						CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions);
	bool						CheckValidationLayerSupport();
	bool						SetupDebugMessenger();
	bool						RunShaderCompiler(const std::string& directoryPath);
	void						PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
	GLFWwindow*					m_pWindow;
	VkInstance					m_vkInstance;
	VulkanRenderer*				m_pVulkanRenderer;	
	VkDebugUtilsMessengerEXT	m_vkDebugMessenger;

	//-----------------------------------------------------------------------------------------------------------------
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
		VkDebugUtilsMessageTypeFlagsEXT msgType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		LOG_ERROR("----------------------------------------------------------------------------------------------------");
		if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			LOG_DEBUG("Validation Layer: {0}", pCallbackData->pMessage);
		}
		else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_WARNING("Validation Layer: {0}", pCallbackData->pMessage);
		}
		else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			LOG_INFO("Validation Layer: {0}", pCallbackData->pMessage);
		}
		else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_CRITICAL("Validation Layer: {0}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}
};

