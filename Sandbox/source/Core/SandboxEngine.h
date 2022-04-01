#pragma once

#include "Core.h"
#include "GLFW/glfw3.h"

class SandboxEngine
{
public:
	SandboxEngine();
	virtual ~SandboxEngine();
	
	void Initialize(GLFWwindow* window);
	void Run(float dt);

private:
	GLFWwindow* m_pWindow;
};
