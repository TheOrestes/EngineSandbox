#pragma once

#include "vulkan/vulkan.h"
#include "Utility.h"

struct  RenderContext;

class VulkanFrameBuffer
{
public:
	VulkanFrameBuffer();
	~VulkanFrameBuffer();

	void								Cleanup(RenderContext* pRC);
	void								CleanupOnWindowsResize(RenderContext* pRC);
	void								HandleWindowResize(RenderContext* pRC);
	bool								CreateFramebuffers(RenderContext* pRC);


private:
	std::vector<Helper::Vulkan::SwapchainAttachment>	m_ListAttachments;
};

