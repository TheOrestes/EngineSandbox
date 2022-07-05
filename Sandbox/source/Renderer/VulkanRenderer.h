#pragma once

#include "vulkan/vulkan.h"
#include "Core/Core.h"
#include "Utility.h"
#include "GLFW/glfw3.h"

#include "IRenderer.h"

class VulkanDevice;

//--- Global Render Context
struct RenderContext
{
	RenderContext()
	{
		pWindow = nullptr;

		vkInst = VK_NULL_HANDLE;
		vkSurface = VK_NULL_HANDLE;
		vkPhysicalDevice = VK_NULL_HANDLE;
		vkDevice = VK_NULL_HANDLE;

		vkSwapchain = VK_NULL_HANDLE;

		vkQueueGraphics = VK_NULL_HANDLE;
		vkQueuePresent = VK_NULL_HANDLE;

		vkForwardRenderingPipeline = VK_NULL_HANDLE;
		vkForwardRenderingPipelineLayout = VK_NULL_HANDLE;
		vkForwardRenderingRenderPass = VK_NULL_HANDLE;

		vkListFramebuffers.clear();
	}

	~RenderContext() 
	{
		pWindow = nullptr;

		vkInst = VK_NULL_HANDLE;
		vkSurface = VK_NULL_HANDLE;
		vkPhysicalDevice = VK_NULL_HANDLE;
		vkDevice = VK_NULL_HANDLE;

		vkSwapchain = VK_NULL_HANDLE;

		vkQueueGraphics = VK_NULL_HANDLE;
		vkQueuePresent = VK_NULL_HANDLE;

		vkForwardRenderingPipeline = VK_NULL_HANDLE;
		vkForwardRenderingPipelineLayout = VK_NULL_HANDLE;
		vkForwardRenderingRenderPass = VK_NULL_HANDLE;

		vkListFramebuffers.clear();
	}

	VkInstance						vkInst;
	GLFWwindow*						pWindow;
	VkSurfaceKHR					vkSurface;
	VkPhysicalDevice				vkPhysicalDevice;
	VkDevice						vkDevice;

	VkSwapchainKHR					vkSwapchain;
	VkExtent2D						vkSwapchainExtent;
	VkFormat						vkSwapchainImageFormat;

	VkQueue							vkQueueGraphics;
	VkQueue							vkQueuePresent;

	VkCommandPool					vkGraphicsCommandPool;
	std::vector<VkCommandBuffer>	vkListGraphicsCommandBuffers;

	VkPipeline						vkForwardRenderingPipeline;
	VkPipelineLayout				vkForwardRenderingPipelineLayout;
	VkRenderPass					vkForwardRenderingRenderPass;

	std::vector<VkFramebuffer>		vkListFramebuffers;
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanRenderer : public IRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool								Initialize(GLFWwindow* pWindow, VkInstance instance);
	void								Update(float dt);
	void								Render();
	bool								RecordCommands();
	bool								CreateSynchronization();
	void								Destroy();

private:
	bool								CreateGraphicsPipeline(Helper::App::ePipeline pipeline);
	bool								CreateRenderPass();

private:
	RenderContext*						m_pRC;
	VulkanDevice*						m_pVulkanDevice;

	// -- Synchronization!
	uint32_t							m_uiCurrentFrame;
	std::vector<VkSemaphore>			m_vkListSemaphoreImageAvailable;
	std::vector<VkSemaphore>			m_vkListSemaphoreRenderFinished;
	std::vector<VkFence>				m_vkListFences;
};

