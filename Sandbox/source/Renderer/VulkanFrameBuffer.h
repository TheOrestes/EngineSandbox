#pragma once

#include "vulkan/vulkan.h"
#include "Utility.h"

struct  VulkanContext;

class VulkanFrameBuffer
{
public:
	VulkanFrameBuffer();
	~VulkanFrameBuffer();

	void								Cleanup(VulkanContext* pRC);
	void								CleanupOnWindowsResize(VulkanContext* pRC);
	void								HandleWindowResize(VulkanContext* pRC);
	bool								CreateFramebuffers(VulkanContext* pRC);


private:
	std::vector<Helper::SwapchainAttachment>	m_ListAttachments;
};

