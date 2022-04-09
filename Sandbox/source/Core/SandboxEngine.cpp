#include "sandboxPCH.h"
#include "SandboxEngine.h"
#include "Logger.h"
#include "VulkanApplication.h"

//---------------------------------------------------------------------------------------------------------------------
SandboxEngine::SandboxEngine()
{
	m_pWindow = nullptr;
	m_pVulkanApp = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
SandboxEngine::~SandboxEngine()
{
	SAFE_DELETE(m_pVulkanApp);

	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

//---------------------------------------------------------------------------------------------------------------------
bool SandboxEngine::Initialize(GLFWwindow* window)
{
	m_pWindow = window;

	m_pVulkanApp = new VulkanApplication();

	int returnCode = m_pVulkanApp->Initialize(reinterpret_cast<void*>(m_pWindow));

	return returnCode;
}   

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::Run(float dt)
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();

		
	}
}
