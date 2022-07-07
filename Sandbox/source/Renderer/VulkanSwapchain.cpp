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
void VulkanSwapchain::Cleanup(const RenderContext* pRC)
{
	vkDestroySwapchainKHR(pRC->vkDevice, pRC->vkSwapchain, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::CleanupOnWindowsResize(const RenderContext* pRC)
{
	vkDestroySwapchainKHR(pRC->vkDevice, pRC->vkSwapchain, nullptr);
	LOG_DEBUG("Swapchain Cleanup on Windows resize");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanSwapchain::HandleWindowsResize(RenderContext* pRC)
{

}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanSwapchain::CreateSwapchain(RenderContext* pRC, const QueueFamilyIndices& queueFamilyIndices)
{
	// 1. Choose best surface format
	VkSurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat(m_SwapchainInfo.surfaceFormats);

	// 2. Choose best presentation format
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_SwapchainInfo.surfacePresentModes);

	// 3. Choose Swapchain image resolution
	VkExtent2D extent = ChooseSwapExtent(pRC);

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

	VkResult result = vkCreateSwapchainKHR(pRC->vkDevice, &swapchainCreateInfo, nullptr, &(pRC->vkSwapchain));

	LOG_DEBUG("Vulkan Swapchain Created!");

	// Save this for later purposes. 
	pRC->vkSwapchainImageFormat = surfaceFormat.format;
	pRC->vkSwapchainExtent = extent;
		 
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// Fetch Surface information on capabilities, Format & Presentation modes which will be used while creating swapchain!

void VulkanSwapchain::FetchSwapchainInfo(VkPhysicalDevice device, VkSurfaceKHR surface)
{
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
		return { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted, search for optimal format
	for (const auto& format : availableFormats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB)
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
VkExtent2D VulkanSwapchain::ChooseSwapExtent(const RenderContext* pRC)
{
	// Get the surface capabilities for a given device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pRC->vkPhysicalDevice, pRC->vkSurface, &m_SwapchainInfo.surfaceCapabilities);

	// The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the 
	// resolution of the window that we're drawing to.The range of the possible resolutions is defined in the 
	// VkSurfaceCapabilitiesKHR structure.Vulkan tells us to match the resolution of the window by setting the 
	// width and height in the currentExtent member.However, some window managers do allow us to differ here 
	// and this is indicated by setting the width and height in currentExtent to a special value : the maximum 
	// value of uint32_t. In that case we'll pick the resolution that best matches the window within the 
	// minImageExtent and maxImageExtent bounds.
	if (m_SwapchainInfo.surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return m_SwapchainInfo.surfaceCapabilities.currentExtent;
	}
	else
	{
		// To handle window resize properly, query current width-height of framebuffer, instead of global value!
		int width, height;
		glfwGetFramebufferSize(pRC->pWindow, &width, &height);

		//VkExtent2D actualExtent = { App::WIDTH, App::HEIGHT };
		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width =  std::clamp(actualExtent.width,  m_SwapchainInfo.surfaceCapabilities.minImageExtent.width, 
										m_SwapchainInfo.surfaceCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, m_SwapchainInfo.surfaceCapabilities.minImageExtent.height, 
										m_SwapchainInfo.surfaceCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}

//---------------------------------------------------------------------------------------------------------------------

