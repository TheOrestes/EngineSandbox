#include "sandboxPCH.h"
#include "VulkanApplication.h"
#include "Core.h"
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::VulkanApplication()
{
}

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::~VulkanApplication()
{
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::Initialize(void* pWindow)
{
	GLFWwindow* window = reinterpret_cast<GLFWwindow*>(pWindow);

	CHECK(CreateInstance());
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Update(float dt)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Render()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Destroy()
{
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Vulkan Rendering";
	appInfo.pEngineName = "Vulkan Sandbox";
	
	VkInstanceCreateInfo instCreateInfo = {};

	VK_CHECK(vkCreateInstance(&instCreateInfo, nullptr, &m_vkInstance));
	
	//std::array<std::string, 2> arrInstanceExtensions = { "VK_KHR_Surface", "VK_KHR_win32_surface" };
	//std::array<std::string, 1> arrEnabledExtensions = { "VK_LAYER_KHRONOS_validation" };

	return true;
}
