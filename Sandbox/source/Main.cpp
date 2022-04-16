
#include "sandboxPCH.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "Core/SandboxEngine.h"

int main(int argc, char** argv)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(gWindowWidht, gWindowHeight, "Vulkan Sandbox", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		LOG_ERROR("Window creation failed!");
	}

	LOG_DEBUG("GLFW Window Created!");

	SandboxEngine MainEngine;
	
	if (!MainEngine.Initialize(window))
		return EXIT_FAILURE;

	MainEngine.Run(0.016f);

	return EXIT_SUCCESS;
}