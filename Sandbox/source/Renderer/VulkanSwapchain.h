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
struct SwapchainImage
{
	VkImage		image;
	VkImageView imageView;
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanSwapchain
{
public:
	VulkanSwapchain();
	~VulkanSwapchain();

	void								Destroy(const RenderContext* pRC);

	bool								CreateSwapchain(const RenderContext* pRC);
	void								FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface);

	inline bool							isSwapchainValid() const { return m_SwapchainInfo.isValid(); } 

private:
	VkSurfaceFormatKHR					ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR					ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D							ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* pWindow);

	bool								CreateImageView(const RenderContext* pRC, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imgView);

private:
	uint32_t							m_uiImageCount;
	SwapchainInfo						m_SwapchainInfo;

	VkSwapchainKHR						m_Swapchain;

public:
	VkFormat							m_SwapchainImageFormat;
	VkExtent2D							m_SwapchainExtent;
	std::vector<SwapchainImage>			m_vecSwapchainImages;
};

