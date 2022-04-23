#include "sandboxPCH.h"
#include "VulkanApplication.h"
#include "Renderer/VulkanRenderer.h"
#include "Renderer/Utility.h"
#include "GLFW/glfw3.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::VulkanApplication()
{
	m_vkDebugMessenger = VK_NULL_HANDLE;
	m_vkInstance = VK_NULL_HANDLE;
	m_pVulkanRenderer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanApplication::~VulkanApplication()
{
	SAFE_DELETE(m_pVulkanRenderer);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::Initialize(void* pWindow)
{
	m_pWindow = reinterpret_cast<GLFWwindow*>(pWindow);

	CHECK(CreateInstance());
	CHECK(SetupDebugMessenger());

	m_pVulkanRenderer = new VulkanRenderer();
	CHECK(m_pVulkanRenderer->Initialize(m_pWindow, m_vkInstance));
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Update(float dt)
{
	m_pVulkanRenderer->Update(dt);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Render()
{
	m_pVulkanRenderer->Render();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::Destroy()
{
	if (Helper::Vulkan::g_bEnableValidationLayer)
	{
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
	}

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
	std::vector<const char*> vecExtensions(glfwExtensions, glfwExtensions + instanceExtnCount);

	if (Helper::Vulkan::g_bEnableValidationLayer)
	{
		// Add this to required application extensions!
		vecExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Check if extensions required by GLFW are supported by our Vulkan instance!
	if (!CheckInstanceExtensionSupport(vecExtensions))
	{
		LOG_ERROR("VkInstance does not support required extensions!!!");
		return false;
	}

	LOG_DEBUG("Extensions supported by Vulkan instance!");

	// Debug Validation layer!
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

	// Supported! Create Vulkan Instance!
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo = &appInfo;
	instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vecExtensions.size());
	instCreateInfo.ppEnabledExtensionNames = vecExtensions.data();

	if (Helper::Vulkan::g_bEnableValidationLayer)
	{
		instCreateInfo.enabledLayerCount = static_cast<uint32_t>(Helper::Vulkan::g_strValidationLayers.size());
		instCreateInfo.ppEnabledLayerNames = Helper::Vulkan::g_strValidationLayers.data();
		instCreateInfo.pNext = nullptr;
	}
	else
	{
		instCreateInfo.enabledLayerCount = 0;
		instCreateInfo.ppEnabledLayerNames = nullptr;
		instCreateInfo.pNext = nullptr;
	}

	VK_CHECK(vkCreateInstance(&instCreateInfo, nullptr, &m_vkInstance));

	LOG_DEBUG("Vulkan Instance created!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::SetupDebugMessenger()
{
	if (!Helper::Vulkan::g_bEnableValidationLayer)
		return false;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	VK_CHECK(CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_vkDebugMessenger));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanApplication::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.flags = 0;
	createInfo.messageSeverity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |	
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> vecExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vecExtensions.data());

#if defined _DEBUG
	// Enumerate all the extensions supported by the vulkan instance.
	// Ideally, this list should contain extensions requested by GLFW and
	// few additional ones!
	LOG_DEBUG("--------- Available Vulkan Extensions ---------");
	for (int i = 0; i < extensionCount; ++i)
	{
		LOG_INFO(vecExtensions[i].extensionName);
	}
	LOG_DEBUG("-----------------------------------------------");
#endif

	// Check if given extensions are in the list of available extensions
	for (uint32_t i = 0; i < extensionCount; i++)
	{
		bool hasExtension = false;

		for (uint32_t j = 0; j < instanceExtensions.size(); j++)
		{
			if (strcmp(vecExtensions[i].extensionName, instanceExtensions[j]))
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
			return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanApplication::CheckValidationLayerSupport()
{
	// Enumerate available validation layers for the vulkan instance!
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> vecAvailableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, vecAvailableLayers.data());

	// Try to see if requested enumeration layers [in Helper.h] is present in available 
	// validation layers. 
	bool layerFound = false;
	for (int i = 0; i < Helper::Vulkan::g_strValidationLayers.size(); ++i)
	{
		for (int j = 0; j < layerCount; ++j)
		{
			if (strcmp(Helper::Vulkan::g_strValidationLayers[i], vecAvailableLayers[j].layerName) == 0)
			{
				layerFound = true;

				std::string msg = std::string(Helper::Vulkan::g_strValidationLayers[i]) + " validation layer found!";
				LOG_DEBUG(msg.c_str());
			}
		}
	}

	return layerFound;
}
