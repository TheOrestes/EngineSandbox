#pragma once

#include "vulkan/vulkan.h"
#include "Core/Core.h"
#include "GLFW/glfw3.h"

class VulkanDevice;

struct RenderContext
{
	RenderContext()
	{
		pWindow = nullptr;
		pVulkanDevice = nullptr;
	}

	~RenderContext()
	{
		SAFE_DELETE(pVulkanDevice);
	}

	GLFWwindow*		pWindow;
	VulkanDevice*	pVulkanDevice;
};

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool				Initialize(GLFWwindow* pWindow, VkInstance instance);
	void				Update(float dt);
	void				Render();
	void				Destroy();

private:
	RenderContext*		m_pRC;
};

