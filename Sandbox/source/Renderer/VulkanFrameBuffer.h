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
	bool								CreateFramebuffersAttachments(VulkanContext* pContext);
	bool								CreateFramebuffers(VulkanContext* pRC);

private:
	bool								CreateDepthBuffer(VulkanContext* pRC);

public:
	std::vector<Helper::SwapchainAttachment>	m_ListAttachments;
	Helper::DepthAttachment						m_depthAttachment;
};

