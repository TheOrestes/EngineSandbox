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
	vkDestroyInstance(m_vkInstance, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::CreateInstance()
{
	// Create basic application information!
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Vulkan Rendering";
	appInfo.pEngineName = "Vulkan Sandbox";

	// Get list of extensions required by GLFW
	uint32_t instanceExtnCount = 0;
	const char** glfwExtensions;
	
	glfwExtensions = glfwGetRequiredInstanceExtensions(&instanceExtnCount);
	std::vector<const char*> vecGlfwExtensions(glfwExtensions, glfwExtensions + instanceExtnCount);

	// Check if extensions required by GLFW are supported by our Vulkan instance!
	if (!CheckInstanceExtensionSupport(vecGlfwExtensions))
	{
		LOG_ERROR("VkInstance does not support required extensions!!!");
		return false;
	}

	LOG_DEBUG("GLFW Extensions supported by Vulkan instance!");

	// Supported! Create Vulkan Instance!
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo = &appInfo;
	instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtnCount);
	instCreateInfo.ppEnabledExtensionNames = vecGlfwExtensions.data();

	// TODO!!
	instCreateInfo.enabledLayerCount = 0;
	instCreateInfo.ppEnabledLayerNames = nullptr;

	VK_CHECK(vkCreateInstance(&instCreateInfo, nullptr, &m_vkInstance));

	LOG_DEBUG("Vulkan Instance created!");
	
	//std::array<std::string, 2> arrInstanceExtensions = { "VK_KHR_Surface", "VK_KHR_win32_surface" };
	//std::array<std::string, 1> arrEnabledExtensions = { "VK_LAYER_KHRONOS_validation" };

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::CheckInstanceExtensionSupport(std::vector<const char*> inExtensions)
{
	// Get number of vulkan extensions
	uint32_t extnCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extnCount, nullptr);

	std::vector<VkExtensionProperties> vecExtensions(extnCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extnCount, vecExtensions.data());

	for (uint16_t i = 0 ; i < inExtensions.size(); i++)
	{
		bool hasExtn = false;
		for (uint16_t j = 0 ; j < vecExtensions.size() ; j++)
		{
			if (!strcmp(inExtensions[i], vecExtensions[j].extensionName))
			{
				hasExtn = true;
				break;
			}
		}
		
		if (!hasExtn)
		{
			return false;
		}
	}

	return true;
}
