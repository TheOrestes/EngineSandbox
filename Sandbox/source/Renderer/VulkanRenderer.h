#pragma once

#include "vulkan/vulkan.h"
#include "Core/Core.h"
#include "Utility.h"
#include "GLFW/glfw3.h"

class VulkanDevice;

//-- Global Render Context
struct RenderContext
{
	RenderContext()
	{
		pWindow = nullptr;
		pVulkanDevice = nullptr;
		vkSurface = VK_NULL_HANDLE;
		vkInst = VK_NULL_HANDLE;
	}

	~RenderContext()
	{
		SAFE_DELETE(pVulkanDevice);
	}

	VkInstance		vkInst;
	GLFWwindow* pWindow;
	VulkanDevice* pVulkanDevice;
	VkSurfaceKHR	vkSurface;
};

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool								Initialize(GLFWwindow* pWindow, VkInstance instance);
	void								Update(float dt);
	void								Render();
	void								Destroy();

private:
	RenderContext*						m_pRC;
};

