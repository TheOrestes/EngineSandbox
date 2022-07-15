#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

struct VulkanContext;

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

	void								Cleanup(VulkanContext* pContext);
	void								CleanupOnWindowsResize(VulkanContext* pContext);
	void								HandleWindowsResize(VulkanContext* pContext);

	bool								CreateSwapchain(VulkanContext* pContext, const QueueFamilyIndices& queueFamilyIndices);
	void								FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface);

	inline bool							isSwapchainValid() const { return m_SwapchainInfo.isValid(); } 

private:
	VkSurfaceFormatKHR					ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR					ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D							ChooseSwapExtent(const VulkanContext* pContext);

private:
	SwapchainInfo						m_SwapchainInfo;
};

