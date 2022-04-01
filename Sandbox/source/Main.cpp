
#include "sandboxPCH.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "Core/SandboxEngine.h"

int main(int argc, char** argv)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(gWindowWidht, gWindowHeight, "Vulkan Sandbox", nullptr, nullptr);

	glm::vec3 position;

	if (!window)
	{
		LOG_CRITICAL("Window creation failed!");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	LOG_DEBUG("GLFW Window Created!");

	SandboxEngine MainEngine;
	MainEngine.Initialize(window);
	MainEngine.Run(0.016f);

	return EXIT_SUCCESS;
}