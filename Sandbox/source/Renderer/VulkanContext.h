#pragma once

#include "vulkan/vulkan.h"
#include "Core/Core.h"
#include "Utility.h"
#include "GLFW/glfw3.h"

class VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

	void								HandleWindowsResize();
	void								Cleanup();
	void								CleanupOnWindowsResize();

	//-- Shader Modules
	VkShaderModule						CreateShaderModule(const std::string& fileName) const;

	//-- Images
	bool								CreateImageView2D(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView) const;

	//-- Buffers
	uint32_t							FindMemoryTypeIndex(uint32_t allowedTypeIndex, VkMemoryPropertyFlags props) const;
	bool								CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memFlags, VkBuffer* outBuffer, VkDeviceMemory* outMemory) const;
	bool								CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize) const;
	VkCommandBuffer						BeginCommandBuffer() const;
	bool								EndAndSubmitCommandBuffer(VkCommandBuffer commandBuffer) const;

public:
	VkInstance							vkInst;
	GLFWwindow* pWindow;
	VkSurfaceKHR						vkSurface;
	VkPhysicalDevice					vkPhysicalDevice;
	VkDevice							vkDevice;
	VkPhysicalDeviceMemoryProperties	vkDeviceMemoryProps;

	uint32_t							uiNumSwapchainImages;
	VkSwapchainKHR						vkSwapchain;
	VkExtent2D							vkSwapchainExtent;
	VkFormat							vkSwapchainImageFormat;

	VkQueue								vkQueueGraphics;
	VkQueue								vkQueuePresent;

	VkCommandPool						vkGraphicsCommandPool;
	std::vector<VkCommandBuffer>		vkListGraphicsCommandBuffers;

	VkPipeline							vkForwardRenderingPipeline;
	VkPipelineLayout					vkForwardRenderingPipelineLayout;
	VkRenderPass						vkForwardRenderingRenderPass;

	std::vector<VkFramebuffer>			vkListFramebuffers;
};

