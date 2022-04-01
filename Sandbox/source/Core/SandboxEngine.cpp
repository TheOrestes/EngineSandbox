#include "sandboxPCH.h"
#include "SandboxEngine.h"
#include "Logger.h"

//---------------------------------------------------------------------------------------------------------------------
SandboxEngine::SandboxEngine()
{
	m_pWindow = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
SandboxEngine::~SandboxEngine()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::Initialize(GLFWwindow* window)
{
	m_pWindow = window;
}   

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::Run(float dt)
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();
	}
}
