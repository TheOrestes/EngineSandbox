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

		vkGraphicsPipeline = VK_NULL_HANDLE;
	}

	~RenderContext() {}

	VkInstance			vkInst;
	GLFWwindow* pWindow;
	VkSurfaceKHR		vkSurface;
	VkPhysicalDevice	vkPhysicalDevice;
	VkDevice			vkDevice;

	VkSwapchainKHR		vkSwapchain;
	VkExtent2D			vkSwapchainExtent;
	VkFormat			vkSwapchainImageFormat;

	VkQueue				vkQueueGraphics;
	VkQueue				vkQueuePresent;

	VkPipeline			vkGraphicsPipeline;
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
	void								Destroy();

private:
	bool								CreateGraphicsPipeline(Helper::App::ePipeline pipeline);

private:
	RenderContext*						m_pRC;
	VulkanDevice*						m_pVulkanDevice;
};

