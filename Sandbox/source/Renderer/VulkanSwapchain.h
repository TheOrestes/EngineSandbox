#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

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

	void								Cleanup(const RenderContext* pRC);
	void								CleanupOnWindowsResize(const RenderContext* pRC);
	void								HandleWindowsResize(RenderContext* pRC);

	bool								CreateSwapchain(RenderContext* pRC, const QueueFamilyIndices& queueFamilyIndices);
	void								FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface);

	inline bool							isSwapchainValid() const { return m_SwapchainInfo.isValid(); } 

private:
	VkSurfaceFormatKHR					ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR					ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D							ChooseSwapExtent(const RenderContext* pRC);

private:
	uint32_t							m_uiImageCount;
	SwapchainInfo						m_SwapchainInfo;
};

