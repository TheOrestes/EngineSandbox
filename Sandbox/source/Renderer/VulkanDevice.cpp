#include "sandboxPCH.h"
#include "Core/Core.h"
#include "VulkanDevice.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface)
{
	m_vkInstance = instance;
	m_vkSurfaceKHR = surface;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::~VulkanDevice()
{
	
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::SetupDevice()
{
	CHECK(AcquirePhysicalDevice());
	CHECK(CreateLogicalDevice());
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

	for (uint16_t i = 0; i < deviceCount; ++i)
	{
		FetchQueueFamilies(vecDevices[i]);
		bool bExtensionsSupported = CheckDeviceExtensionSupport(vecDevices[i]);

		if (m_QueueFamilyIndices.isComplete() && bExtensionsSupported)
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
	// Queue the logical device needs to create & the info to do so!
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	float priority = 1.0f;
	queueCreateInfo.pQueuePriorities = &priority;
	
	// Information needed to create logical device!
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
	
	// Physical device features that logical device will use...
	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &deviceFeatures);

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create logical device from the given physical device...
	VK_CHECK(vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkLogicalDevice));

	LOG_DEBUG("Vulkan Logical device created!");

	// Queues are created at the same time as device creation, store their handle!
	vkGetDeviceQueue(m_vkLogicalDevice, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_vkQueueGraphics);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::Destroy()
{
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
		//VkBool32 bPresentSupport = false;
		//vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_vkSurfaceKHR, &bPresentSupport);
		//
		//// if yes, store presentation family queue index!
		//if (bPresentSupport)
		//{
		//	m_QueueFamilyIndices.presentFamily = i;
		//}

		if (m_QueueFamilyIndices.isComplete())
			break;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	return true;
}
