#pragma once

#include "Core.h"
#include "GLFW/glfw3.h"

class VulkanApplication;

class SandboxEngine
{
public:
	SandboxEngine();
	virtual ~SandboxEngine();
	
	bool Initialize(GLFWwindow* window);
	void Run(float dt);

private:
	GLFWwindow*				m_pWindow;
	VulkanApplication*		m_pVulkanApp;
};
