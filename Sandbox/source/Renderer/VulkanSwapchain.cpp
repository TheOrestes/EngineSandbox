#include "sandboxPCH.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderer.h"
#include "Core/Core.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanSwapchain::VulkanSwapchain()
{
}

//---------------------------------------------------------------------------------------------------------------------
VulkanSwapchain::~VulkanSwapchain()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::Destroy(const RenderContext* pRC)
{
	for (SwapchainImage image : m_vecSwapchainImages)
	{
		vkDestroyImageView(pRC->vkDevice, image.imageView, nullptr);
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanSwapchain::CreateSwapchain(RenderContext* pRC, const QueueFamilyIndices& queueFamilyIndices)
{
	// 1. Choose best surface format
	VkSurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat(m_SwapchainInfo.surfaceFormats);

	// 2. Choose best presentation format
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_SwapchainInfo.surfacePresentModes);

	// 3. Choose Swapchain image resolution
	VkExtent2D extent = ChooseSwapExtent(m_SwapchainInfo.surfaceCapabilities, pRC->pWindow);

	// decide how many images to have in the swap chain, it's good practice to have an extra count.
	// Also make sure it does not exceed maximum number of images
	uint32_t minImageCount = m_SwapchainInfo.surfaceCapabilities.minImageCount;
	m_uiImageCount = minImageCount + 1;
	if (m_SwapchainInfo.surfaceCapabilities.maxImageCount > 0 && m_uiImageCount > m_SwapchainInfo.surfaceCapabilities.maxImageCount)
	{
		m_uiImageCount = m_SwapchainInfo.surfaceCapabilities.maxImageCount;
	}

	// Swapchain creation info
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = pRC->vkSurface;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.minImageCount = m_uiImageCount;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = m_SwapchainInfo.surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
	{
		uint32_t indices[] =
		{
			queueFamilyIndices.graphicsFamily.value(),
			queueFamilyIndices.presentFamily.value()
		};

		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = indices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(pRC->vkDevice, &swapchainCreateInfo, nullptr, &(pRC->vkSwapchain)));

	LOG_DEBUG("Vulkan Swapchain Created!");

	// Save this for later purposes. 
	pRC->vkSwapchainImageFormat = surfaceFormat.format;
	pRC->vkSwapchainExtent = extent;

	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, nullptr);

	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, images.data());

	for (VkImage image : images)
	{
		// store image handle
		SwapchainImage swapchainImage= {};
		swapchainImage.image = image;

		// Create Image View
		CreateImageView(pRC, image, pRC->vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, &(swapchainImage.imageView));

		// Add to the list...
		m_vecSwapchainImages.push_back(swapchainImage);
	}

	LOG_DEBUG("Swapchain Images & Imageviews created");
		 
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// Fetch Surface information on capabilities, Format & Presentation modes which will be used while creating swapchain!

void VulkanSwapchain::FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	// Get the surface capabilities for a given device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &m_SwapchainInfo.surfaceCapabilities);

	// Get the formats
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,nullptr);

	if (formatCount != 0)
	{
		m_SwapchainInfo.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_SwapchainInfo.surfaceFormats.data());
	}

	// Presentation modes
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

	if (presentationCount != 0)
	{
		m_SwapchainInfo.surfacePresentModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, m_SwapchainInfo.surfacePresentModes.data());
	}
}

//---------------------------------------------------------------------------------------------------------------------
VkSurfaceFormatKHR VulkanSwapchain::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// If only 1 format available and is undefined, then this means ALL formats are available (no restrictions)
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted, search for optimal format
	for (const auto& format : availableFormats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return availableFormats[0];
}

//---------------------------------------------------------------------------------------------------------------------
VkPresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	// out of all Mailbox allows triple buffering, so if available use it, else use FIFO mode.
	for (uint32_t i = 0; i < availablePresentModes.size(); ++i)
	{
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentModes[i];
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

//---------------------------------------------------------------------------------------------------------------------
VkExtent2D VulkanSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* pWindow)
{
	// The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the 
	// resolution of the window that we're drawing to.The range of the possible resolutions is defined in the 
	// VkSurfaceCapabilitiesKHR structure.Vulkan tells us to match the resolution of the window by setting the 
	// width and height in the currentExtent member.However, some window managers do allow us to differ here 
	// and this is indicated by setting the width and height in currentExtent to a special value : the maximum 
	// value of uint32_t. In that case we'll pick the resolution that best matches the window within the 
	// minImageExtent and maxImageExtent bounds.
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		// To handle window resize properly, query current width-height of framebuffer, instead of global value!
		int width, height;
		glfwGetFramebufferSize(pWindow, &width, &height);

		//VkExtent2D actualExtent = { App::WIDTH, App::HEIGHT };
		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanSwapchain::CreateImageView(const RenderContext* pRC, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.format = format;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(pRC->vkDevice, &createInfo, nullptr, imageView));

	return true;
}
