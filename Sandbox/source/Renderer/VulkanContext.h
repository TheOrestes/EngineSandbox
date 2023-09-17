#pragma once

#include "vulkan/vulkan.h"
#include "Core/Core.h"
#include "Utility.h"
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
enum class ERenderer
{
	RASTERIZER = 0,
	SOFTWARE_RT,
	HARDWARE_RT
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();

	void								Cleanup();
	void								CleanupOnWindowsResize();

	//-- Shader Modules
	VkShaderModule						CreateShaderModule(const std::string& fileName) const;

	//-- Images
	VkFormat							ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags) const;
	bool								CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags,
													VkMemoryPropertyFlags memoryPropertyFlags, VkImage* pImage, VkDeviceMemory* pDeviceMemory) const;
	bool								CreateImageView2D(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView) const;
	bool								CopyImageBuffer(VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height) const;
	void								TransitionImageLayout(VkImage srcImage, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE) const;

	//-- Buffers
	uint32_t							FindMemoryTypeIndex(uint32_t allowedTypeIndex, VkMemoryPropertyFlags props) const;
	bool								CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memFlags, VkBuffer* outBuffer, VkDeviceMemory* outMemory) const;
	bool								CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize) const;
	VkCommandBuffer						BeginCommandBuffer() const;
	bool								EndAndSubmitCommandBuffer(VkCommandBuffer commandBuffer) const;

public:
	ERenderer							eRendererMode;

	VkInstance							vkInst;
	GLFWwindow*							pWindow;
	VkSurfaceKHR						vkSurface;
	VkPhysicalDevice					vkPhysicalDevice;
	VkDevice							vkDevice;
	VkPhysicalDeviceMemoryProperties	vkDeviceMemoryProps;

	uint32_t							uiNumSwapchainImages;
	VkSwapchainKHR						vkSwapchain;
	VkExtent2D							vkSwapchainExtent;
	VkFormat							vkSwapchainImageFormat;
	VkFormat							vkDepthImageFormat;

	VkQueue								vkQueueGraphics;
	VkQueue								vkQueuePresent;

	VkCommandPool						vkGraphicsCommandPool;
	std::vector<VkCommandBuffer>		vkListGraphicsCommandBuffers;

	VkPipeline							vkForwardRenderingPipeline;
	VkPipelineLayout					vkForwardRenderingPipelineLayout;
	VkRenderPass						vkForwardRenderingRenderPass;

	std::vector<VkFramebuffer>			vkListFramebuffers;
};

