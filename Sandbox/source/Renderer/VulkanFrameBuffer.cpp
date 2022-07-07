#include "sandboxPCH.h"
#include "VulkanRenderer.h"
#include "VulkanFrameBuffer.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanFrameBuffer::VulkanFrameBuffer()
{
	m_ListAttachments.clear();
}

//---------------------------------------------------------------------------------------------------------------------
VulkanFrameBuffer::~VulkanFrameBuffer()
{
	m_ListAttachments.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanFrameBuffer::Cleanup(RenderContext* pRC)
{
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		vkDestroyFramebuffer(pRC->vkDevice, pRC->vkListFramebuffers[i], nullptr);
		vkDestroyImageView(pRC->vkDevice, m_ListAttachments[i].imageView, nullptr);
	}

	m_ListAttachments.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanFrameBuffer::CleanupOnWindowsResize(RenderContext* pRC)
{
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		vkDestroyFramebuffer(pRC->vkDevice, pRC->vkListFramebuffers[i], nullptr);
		vkDestroyImageView(pRC->vkDevice, m_ListAttachments[i].imageView, nullptr);
	}

	m_ListAttachments.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanFrameBuffer::HandleWindowResize(RenderContext* pRC)
{
	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, nullptr);

	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, images.data());

	for (VkImage image : images)
	{
		// store image handle
		Helper::Vulkan::SwapchainAttachment swapchainImage = {};
		swapchainImage.image = image;

		// Create Image View
		Helper::Vulkan::CreateImageView2D(pRC->vkDevice, image, pRC->vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, &(swapchainImage.imageView));

		// Add to the list...
		m_ListAttachments.push_back(swapchainImage);
	}

	pRC->vkListFramebuffers.resize(m_ListAttachments.size());

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		std::array<VkImageView, 1> attachments =
		{
			m_ListAttachments[i].imageView
		};

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = pRC->vkForwardRenderingRenderPass;
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCreateInfo.pAttachments = attachments.data();
		fbCreateInfo.width = pRC->vkSwapchainExtent.width;
		fbCreateInfo.height = pRC->vkSwapchainExtent.height;
		fbCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(pRC->vkDevice, &fbCreateInfo, nullptr, &(pRC->vkListFramebuffers[i]));
		if (result != VK_SUCCESS)
		{
			LOG_ERROR("Failed to Re-Create Framebuffer on Windows Resize!");
			return;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanFrameBuffer::CreateFramebuffers(RenderContext* pRC)
{
	uint32_t swapchainImageCount;
	VK_CHECK(vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, nullptr));

	std::vector<VkImage> images(swapchainImageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(pRC->vkDevice, pRC->vkSwapchain, &swapchainImageCount, images.data()));

	for (VkImage image : images)
	{
		// store image handle
		Helper::Vulkan::SwapchainAttachment swapchainImage = {};
		swapchainImage.image = image;

		// Create Image View
		Helper::Vulkan::CreateImageView2D(pRC->vkDevice, image, pRC->vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, &(swapchainImage.imageView));

		// Add to the list...
		m_ListAttachments.push_back(swapchainImage);
	}

	LOG_DEBUG("Framebuffer attachments created");

	// Check if RenderPass exists!
	if (pRC->vkForwardRenderingRenderPass == VK_NULL_HANDLE)
	{
		LOG_ERROR("Renderpass not set!!!");
		return false;
	}

	pRC->vkListFramebuffers.resize(m_ListAttachments.size());

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		std::array<VkImageView, 1> attachments =
		{
			m_ListAttachments[i].imageView
		};

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = pRC->vkForwardRenderingRenderPass;
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCreateInfo.pAttachments = attachments.data();
		fbCreateInfo.width = pRC->vkSwapchainExtent.width;
		fbCreateInfo.height = pRC->vkSwapchainExtent.height;
		fbCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(pRC->vkDevice, &fbCreateInfo, nullptr, &(pRC->vkListFramebuffers[i])));
	}

	LOG_DEBUG("Framebuffers created!");

	return true;
}
