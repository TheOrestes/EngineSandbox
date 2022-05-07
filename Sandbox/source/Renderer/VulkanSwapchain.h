#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.h"

class VulkanDevice;
struct RenderContext;

//---------------------------------------------------------------------------------------------------------------------
struct SwapchainInfo
{
	VkSurfaceCapabilitiesKHR		surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR>	surfacePresentModes;

	bool isValid() const { return !surfaceFormats.empty() && !surfacePresentModes.empty(); }
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanSwapchain
{
public:
	VulkanSwapchain();
	~VulkanSwapchain();

	void								Destroy(VulkanDevice* pDevice);

	bool								CreateSwapchain(const RenderContext* pRC);
	void								FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface);

	inline bool							isSwapchainValid() const { return m_SwapchainInfo.isValid(); } 

private:
	VkSurfaceFormatKHR					ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR					ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D							ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* pWindow);

private:
	uint32_t							m_uiImageCount;
	SwapchainInfo						m_SwapchainInfo;

	VkSwapchainKHR						m_SwapChain;
};

