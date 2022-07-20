#include "sandboxPCH.h"
#include "VulkanRenderer.h"
#include "VulkanFrameBuffer.h"
#include "VulkanContext.h"

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
void VulkanFrameBuffer::Cleanup(VulkanContext* pContext)
{
	vkDestroyImageView(pContext->vkDevice, m_depthAttachment.imageView, nullptr);
	vkDestroyImage(pContext->vkDevice, m_depthAttachment.image, nullptr);
	vkFreeMemory(pContext->vkDevice, m_depthAttachment.deviceMemory, nullptr);

	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		vkDestroyFramebuffer(pContext->vkDevice, pContext->vkListFramebuffers[i], nullptr);
		vkDestroyImageView(pContext->vkDevice, m_ListAttachments[i].imageView, nullptr);
	}

	m_ListAttachments.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanFrameBuffer::CleanupOnWindowsResize(VulkanContext* pContext)
{
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		vkDestroyFramebuffer(pContext->vkDevice, pContext->vkListFramebuffers[i], nullptr);
		vkDestroyImageView(pContext->vkDevice, m_ListAttachments[i].imageView, nullptr);
	}

	m_ListAttachments.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanFrameBuffer::HandleWindowResize(VulkanContext* pContext)
{
	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(pContext->vkDevice, pContext->vkSwapchain, &swapchainImageCount, nullptr);

	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(pContext->vkDevice, pContext->vkSwapchain, &swapchainImageCount, images.data());

	for (VkImage image : images)
	{
		// store image handle
		Helper::SwapchainAttachment swapchainImage = {};
		swapchainImage.image = image;

		// Create Image View
		pContext->CreateImageView2D(image, pContext->vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, &(swapchainImage.imageView));

		// Add to the list...
		m_ListAttachments.push_back(swapchainImage);
	}

	pContext->vkListFramebuffers.resize(m_ListAttachments.size());

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		std::array<VkImageView, 1> attachments =
		{
			m_ListAttachments[i].imageView
		};

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = pContext->vkForwardRenderingRenderPass;
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCreateInfo.pAttachments = attachments.data();
		fbCreateInfo.width = pContext->vkSwapchainExtent.width;
		fbCreateInfo.height = pContext->vkSwapchainExtent.height;
		fbCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(pContext->vkDevice, &fbCreateInfo, nullptr, &(pContext->vkListFramebuffers[i]));
		if (result != VK_SUCCESS)
		{
			LOG_ERROR("Failed to Re-Create Framebuffer on Windows Resize!");
			return;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanFrameBuffer::CreateFramebuffersAttachments(VulkanContext* pContext)
{
	uint32_t swapchainImageCount;
	VK_CHECK(vkGetSwapchainImagesKHR(pContext->vkDevice, pContext->vkSwapchain, &swapchainImageCount, nullptr));

	std::vector<VkImage> images(swapchainImageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(pContext->vkDevice, pContext->vkSwapchain, &swapchainImageCount, images.data()));

	// Save it to the Context!
	pContext->uiNumSwapchainImages = swapchainImageCount;

	for (VkImage image : images)
	{
		// store image handle
		Helper::SwapchainAttachment swapchainImage = {};
		swapchainImage.image = image;

		// Create Image View
		pContext->CreateImageView2D(image, pContext->vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, &(swapchainImage.imageView));

		// Add to the list...
		m_ListAttachments.push_back(swapchainImage);
	}

	// Create Depth buffer attachment!
	CHECK(CreateDepthBuffer(pContext));

	LOG_DEBUG("Framebuffer attachments created");
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanFrameBuffer::CreateFramebuffers(VulkanContext* pContext)
{
	// Check if RenderPass exists!
	if (pContext->vkForwardRenderingRenderPass == VK_NULL_HANDLE)
	{
		LOG_ERROR("Renderpass not set!!!");
		return false;
	}

	pContext->vkListFramebuffers.resize(m_ListAttachments.size());

	// create framebuffer for each swapchain image
	for (uint32_t i = 0; i < m_ListAttachments.size(); i++)
	{
		std::array<VkImageView, 2> attachments =
		{
			m_ListAttachments[i].imageView,
			m_depthAttachment.imageView
		};

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = pContext->vkForwardRenderingRenderPass;
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCreateInfo.pAttachments = attachments.data();
		fbCreateInfo.width = pContext->vkSwapchainExtent.width;
		fbCreateInfo.height = pContext->vkSwapchainExtent.height;
		fbCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(pContext->vkDevice, &fbCreateInfo, nullptr, &(pContext->vkListFramebuffers[i])));
	}

	LOG_DEBUG("Framebuffers created!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanFrameBuffer::CreateDepthBuffer(VulkanContext* pRC)
{
	// List of depth formats we need
	std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };

	// Choose the supported format
	VkFormat chosenFormat = pRC->ChooseSupportedFormat(depthFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	pRC->vkDepthImageFormat = chosenFormat;

	// Create depth image
	CHECK(pRC->CreateImage2D(	pRC->vkSwapchainExtent.width,
								pRC->vkSwapchainExtent.height,
								chosenFormat, VK_IMAGE_TILING_OPTIMAL, 
								VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
								&m_depthAttachment.image, &m_depthAttachment.deviceMemory));

	
	CHECK(pRC->CreateImageView2D(m_depthAttachment.image, chosenFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depthAttachment.imageView));

	return true;
}
