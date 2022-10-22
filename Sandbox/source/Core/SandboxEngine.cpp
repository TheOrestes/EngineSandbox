#include "sandboxPCH.h"
#include "SandboxEngine.h"
#include "Logger.h"
#include "VulkanApplication.h"
#include "World/Camera.h"

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

	glfwSetWindowUserPointer(m_pWindow, m_pVulkanApp);

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
	glfwSetWindowShouldClose(pWindow, true);
	LOG_DEBUG("Window Closed!");
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::WindowResizedCallback(GLFWwindow* pWindow, int width, int height)
{
	VulkanApplication* pApp = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(pWindow));
	pApp->HandleWindowResizedCallback(pWindow);

	//m_pVulkanApp->HandleWindowResizedCallback(pWindow, width, height);
	LOG_DEBUG("Window Resized to [{0}, {1}]", width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::KeyHandlerCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
	VulkanApplication* pApp = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(pWindow));

	if ((action == GLFW_REPEAT || GLFW_PRESS))
	{
		switch (key)
		{
			case GLFW_KEY_W:
			{
				pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_FORWARD);
				break;
			}

			case GLFW_KEY_S:
			{
				pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_BACK);
				break;
			}

			case GLFW_KEY_A:
			{
				pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_LEFT);
				break;
			}

			case GLFW_KEY_D:
			{
				pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_RIGHT);
				break;
			}

			case GLFW_KEY_ESCAPE:
			{
				glfwSetWindowShouldClose(pWindow, true);
				break;
			}
		}
	}
	
	// Stop if key is released...
	if (action == GLFW_RELEASE)
	{
		pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_NONE);
	}

	//LOG_INFO("{0} Key pressed...", key);
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::MousePositionCallback(GLFWwindow* pWindow, double xPos, double yPos)
{
	VulkanApplication* pApp = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(pWindow));

	// Rotate only when RIGHT CLICK is down!
	if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_PAN_2D, static_cast<float>(xPos), static_cast<float>(yPos), true);
	}
	else
	{
		pApp->HandleSceneInput(pWindow, CameraAction::CAMERA_PAN_2D, static_cast<float>(xPos), static_cast<float>(yPos), false);
	}

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
