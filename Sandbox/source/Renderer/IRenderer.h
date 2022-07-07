#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class IRenderer
{
public:
	IRenderer() {}
	virtual ~IRenderer() {}

	virtual bool	Initialize(GLFWwindow* pWindow, VkInstance instance) = 0;
	virtual void	Update(float dt) = 0;
	virtual void	Render() = 0;
	virtual void	HandleWindowsResize() = 0;
	virtual void	Cleanup() = 0;
	virtual void	CleanupOnWindowsResize() = 0;
};