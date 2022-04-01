
#include "sandboxPCH.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "Core/SandboxEngine.h"

int main(int argc, char** argv)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(gWindowWidht, gWindowHeight, "Vulkan Sandbox", nullptr, nullptr);

	glm::vec3 position;

	if (!window)
	{
		LOG_CRITICAL("Window creation failed!");
		glfwTerminate();
		return -1;
	}

	LOG_DEBUG("GLFW Window Created!");

	SandboxEngine MainEngine;
	MainEngine.Initialize(window);
	MainEngine.Run(0.016f);

	return 0;
}