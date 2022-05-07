#include "sandboxPCH.h"
#include "Core/Core.h"
#include "VulkanRenderer.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "Utility.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::VulkanDevice(const RenderContext* pRC)
{
	m_vkInstance = pRC->vkInst;
	m_vkSurfaceKHR = pRC->vkSurface;

	m_pSwapchain = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::~VulkanDevice()
{
	
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::SetupDevice(const RenderContext* pRC)
{
	CHECK(AcquirePhysicalDevice());
	CHECK(CreateLogicalDevice());
	CHECK(m_pSwapchain->CreateSwapchain(pRC));
}

//---------------------------------------------------------------------------------------------------------------------
// List out all the available physical devices. Choose the one which supports required Queue families & extensions. 
// Give preference to Discrete GPU over Integrated GPU!

bool VulkanDevice::AcquirePhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		LOG_CRITICAL("Can't find GPUs supporting Vulkan!!!");
		return false;
	}		

	std::vector<VkPhysicalDevice> vecDevices(deviceCount);
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, vecDevices.data());

	// List out all the physical devices & get their properties
	for (uint16_t i = 0; i < deviceCount; ++i)
	{
		vkGetPhysicalDeviceProperties(vecDevices[i], &m_vkDeviceProps);
		LOG_INFO("{0} Detected", m_vkDeviceProps.deviceName);
	}

	m_pSwapchain = new VulkanSwapchain();

	for (uint16_t i = 0; i < deviceCount; ++i)
	{
		bool bExtensionsSupported = CheckDeviceExtensionSupport(vecDevices[i]);

		if (bExtensionsSupported)
		{
			// Fetch Queue families supported!
			FetchQueueFamilies(vecDevices[i]);

			// Fetch if surface has required parameters to create swapchain!
			m_pSwapchain->FetchSwapchainInfo(vecDevices[i], m_vkSurfaceKHR);

			if (m_QueueFamilyIndices.isComplete() && m_pSwapchain->isSwapchainValid())
			{
				vkGetPhysicalDeviceProperties(vecDevices[i], &m_vkDeviceProps);

				// Prefer Discrete GPU over integrated one!
				if (m_vkDeviceProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					m_vkPhysicalDevice = vecDevices[i];

					LOG_DEBUG("{0} Selected!!", m_vkDeviceProps.deviceName);
					LOG_INFO("---------- Device Limits ----------");
					LOG_INFO("Max Color Attachments: {0}", m_vkDeviceProps.limits.maxColorAttachments);
					LOG_INFO("Max Descriptor Set Samplers: {0}", m_vkDeviceProps.limits.maxDescriptorSetSamplers);
					LOG_INFO("Max Descriptor Set Uniform Buffers: {0}", m_vkDeviceProps.limits.maxDescriptorSetUniformBuffers);
					LOG_INFO("Max Framebuffer Height: {0}", m_vkDeviceProps.limits.maxFramebufferHeight);
					LOG_INFO("Max Framebuffer Width: {0}", m_vkDeviceProps.limits.maxFramebufferWidth);
					LOG_INFO("Max Push Constant Size: {0}", m_vkDeviceProps.limits.maxPushConstantsSize);
					LOG_INFO("Max Uniform Buffer Range: {0}", m_vkDeviceProps.limits.maxUniformBufferRange);
					LOG_INFO("Max Vertex Input Attributes: {0}", m_vkDeviceProps.limits.maxVertexInputAttributes);

					break;
				}
				else
				{
					LOG_ERROR("{0} Rejected!!", m_vkDeviceProps.deviceName);
				}
			}
		}
		
	}

	// If we don't find suitable device, return!
	if (m_vkPhysicalDevice == VK_NULL_HANDLE)
	{
		LOG_ERROR("Failed to find suitable GPU!");
		return false;
	}
	
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// Create logical device supporting required Queues, Extensions & Device features!

bool VulkanDevice::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	// std::set allows only One unique value for input values, no duplicate is allowed, so if both Graphics Queue family
	// and Presentation Queue family index is same then it will avoid the duplicates and assign only one queue index!
	std::set<uint32_t> uniqueQueueFamilies =
	{
		m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value()
	};

	float queuePriority = 1.0f;

	for (uint32_t queueFamily = 0; queueFamily < uniqueQueueFamilies.size(); ++queueFamily)
	{
		// Queue the logical device needs to create & the info to do so!
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}
	
	// Information needed to create logical device!
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Helper::Vulkan::g_strDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = Helper::Vulkan::g_strDeviceExtensions.data();
	
	// Physical device features that logical device will use...
	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &deviceFeatures);

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create logical device from the given physical device...
	VK_CHECK(vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkLogicalDevice));

	LOG_DEBUG("Vulkan Logical device created!");

	// Queues are created at the same time as device creation, store their handle!
	vkGetDeviceQueue(m_vkLogicalDevice, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_vkQueueGraphics);
	vkGetDeviceQueue(m_vkLogicalDevice, m_QueueFamilyIndices.presentFamily.value(), 0, &m_vkQueuePresent);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::Destroy()
{
	SAFE_DELETE(m_pSwapchain);

	vkDestroyDevice(m_vkLogicalDevice, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
// For a given physical device, checks if it has Queue families which support Graphics & Present family queues!

void VulkanDevice::FetchQueueFamilies(VkPhysicalDevice physicalDevice)
{
	// Get all queue families & their properties supported by physical device!
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> vecQueueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, vecQueueFamilies.data());

	// Go through queue families and check if it supports graphics & present family queue!
	for (int i = 0; i < queueFamilyCount; ++i)
	{
		if (vecQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_QueueFamilyIndices.graphicsFamily = i;
		}

		// check if this queue family has capability of presenting to our window surface!
		VkBool32 bPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_vkSurfaceKHR, &bPresentSupport);

		// if yes, store presentation family queue index!
		if (bPresentSupport)
		{
			m_QueueFamilyIndices.presentFamily = i;
		}

		if (m_QueueFamilyIndices.isComplete())
			break;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Get count of total number of extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// gather their information
	m_vecSupportedExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, m_vecSupportedExtensions.data());

	// Compare Required extensions with supported extensions...
	for (int i = 0; i < Helper::Vulkan::g_strDeviceExtensions.size(); ++i)
	{
		bool bExtensionFound = false;

		for (int j = 0; j < extensionCount; ++j)
		{
			// If device supported extensions matches the one we want, good news ... Enumarate them!
			if (strcmp(Helper::Vulkan::g_strDeviceExtensions[i], m_vecSupportedExtensions[j].extensionName) == 0)
			{
				bExtensionFound = true;

				std::string msg = std::string(Helper::Vulkan::g_strDeviceExtensions[i]) + " device extension found!";
				LOG_DEBUG(msg.c_str());

				break;
			}
		}

		// No matching extension found ... bail out!
		if (!bExtensionFound)
		{
			return false;
		}
	}

	return true;
}

