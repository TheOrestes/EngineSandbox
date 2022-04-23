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

	// Register Events!
	glfwSetWindowCloseCallback(m_pWindow, WindowClosedCallback);
	glfwSetWindowSizeCallback(m_pWindow, WindowResizedCallback);
	glfwSetKeyCallback(m_pWindow, KeyHandlerCallback);
	glfwSetCursorPosCallback(m_pWindow, MousePositionCallback);
	glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
	glfwSetScrollCallback(m_pWindow, MouseScrollCallback);

	return returnCode;
}   

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::Run(float dt)
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();

		m_pVulkanApp->Update(dt);
		m_pVulkanApp->Render();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::WindowClosedCallback(GLFWwindow* pWindow)
{
	LOG_DEBUG("Window Closed!");
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::WindowResizedCallback(GLFWwindow* pWindow, int width, int height)
{
	LOG_DEBUG("Window Resized to [{0}, {1}]", width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::KeyHandlerCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
	//LOG_INFO("{0} Key pressed...", key);
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::MousePositionCallback(GLFWwindow* pWindow, double xPos, double yPos)
{
	//LOG_INFO("Mouse Position = [{0}, {1}]", xPos, yPos);
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods)
{
	//LOG_INFO("{0} Mouse button pressed...", button);
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::MouseScrollCallback(GLFWwindow* pWindow, double xOffset, double yOffset)
{
	//LOG_INFO("Mouse scroll [{0}, {1}]", xOffset, yOffset);
}
