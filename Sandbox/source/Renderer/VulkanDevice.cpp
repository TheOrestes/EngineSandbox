#include "sandboxPCH.h"
#include "Core/Core.h"
#include "VulkanRenderer.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::VulkanDevice(const VulkanContext* pContext)
{
	m_pSwapchain = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanDevice::~VulkanDevice()
{
	SAFE_DELETE(m_pSwapchain);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::SetupDevice(VulkanContext* pContext)
{
	CHECK(AcquirePhysicalDevice(pContext));
	CHECK(CreateLogicalDevice(pContext));
	CHECK(m_pSwapchain->CreateSwapchain(pContext, m_QueueFamilyIndices));
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CreateCommandPool(VulkanContext* pContext)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;			// create command buffer which is reset everytime
	poolInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value();
	
	VK_CHECK(vkCreateCommandPool(pContext->vkDevice, &poolInfo, nullptr, &(pContext->vkGraphicsCommandPool)));

	LOG_DEBUG("Graphics Command Pool created!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CreateCommandBuffers(VulkanContext* pContext)
{
	// Make sure we have command buffer for each framebuffer!
	pContext->vkListGraphicsCommandBuffers.resize(pContext->vkListFramebuffers.size());

	// Allocate buffer from the Graphics command pool
	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = pContext->vkGraphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(pContext->vkListGraphicsCommandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(pContext->vkDevice, &cbAllocInfo, pContext->vkListGraphicsCommandBuffers.data()));

	LOG_DEBUG("Graphics Command buffers created!");

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// List out all the available physical devices. Choose the one which supports required Queue families & extensions. 
// Give preference to Discrete GPU over Integrated GPU!

bool VulkanDevice::AcquirePhysicalDevice(VulkanContext* pContext)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(pContext->vkInst, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		LOG_CRITICAL("Can't find GPUs supporting Vulkan!!!");
		return false;
	}		

	std::vector<VkPhysicalDevice> vecDevices(deviceCount);
	vkEnumeratePhysicalDevices(pContext->vkInst, &deviceCount, vecDevices.data());

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
			FetchQueueFamilies(vecDevices[i], pContext);

			// Fetch if surface has required parameters to create swapchain!
			m_pSwapchain->FetchSwapchainInfo(vecDevices[i], pContext->vkSurface);

			if (m_QueueFamilyIndices.isComplete() && m_pSwapchain->isSwapchainValid())
			{
				vkGetPhysicalDeviceProperties(vecDevices[i], &m_vkDeviceProps);

				// Prefer Discrete GPU over integrated one!
				if (m_vkDeviceProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					pContext->vkPhysicalDevice = vecDevices[i];

					// Get properties of physical device memory
					vkGetPhysicalDeviceMemoryProperties(pContext->vkPhysicalDevice, &(pContext->vkDeviceMemoryProps));

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
	if (pContext->vkPhysicalDevice == VK_NULL_HANDLE)
	{
		LOG_ERROR("Failed to find suitable GPU!");
		return false;
	}
	
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// Create logical device supporting required Queues, Extensions & Device features!

bool VulkanDevice::CreateLogicalDevice(VulkanContext* pContext)
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
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Helper::g_strDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = Helper::g_strDeviceExtensions.data();
	
	// Physical device features that logical device will use...
	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(pContext->vkPhysicalDevice, &deviceFeatures);

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create logical device from the given physical device...
	VK_CHECK(vkCreateDevice(pContext->vkPhysicalDevice, &deviceCreateInfo, nullptr, &(pContext->vkDevice)));

	LOG_DEBUG("Vulkan Logical device created!");

	// Queues are created at the same time as device creation, store their handle!
	vkGetDeviceQueue(pContext->vkDevice, m_QueueFamilyIndices.graphicsFamily.value(), 0, &(pContext->vkQueueGraphics));
	vkGetDeviceQueue(pContext->vkDevice, m_QueueFamilyIndices.presentFamily.value(), 0, &(pContext->vkQueuePresent));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::Cleanup(VulkanContext* pContext)
{
	vkFreeCommandBuffers(pContext->vkDevice, pContext->vkGraphicsCommandPool,
						static_cast<uint32_t>(pContext->vkListGraphicsCommandBuffers.size()),
						pContext->vkListGraphicsCommandBuffers.data());
	vkDestroyCommandPool(pContext->vkDevice, pContext->vkGraphicsCommandPool, nullptr);
	m_pSwapchain->Cleanup(pContext);
	vkDestroyDevice(pContext->vkDevice, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::CleanupOnWindowsResize(VulkanContext* pContext)
{
	m_pSwapchain->CleanupOnWindowsResize(pContext);

	// clean-up existing command buffer & reuse existing pool to allocate new command buffers instead of recreating it!
	vkFreeCommandBuffers(pContext->vkDevice, pContext->vkGraphicsCommandPool, 
						static_cast<uint32_t>(pContext->vkListGraphicsCommandBuffers.size()), 
						pContext->vkListGraphicsCommandBuffers.data());

	LOG_DEBUG("Vulkan Device Cleanup on Windows resize");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanDevice::HandleWindowsResize(VulkanContext* pContext)
{
	m_pSwapchain->CreateSwapchain(pContext, m_QueueFamilyIndices);
}

//---------------------------------------------------------------------------------------------------------------------
// For a given physical device, checks if it has Queue families which support Graphics & Present family queues!

void VulkanDevice::FetchQueueFamilies(VkPhysicalDevice physicalDevice, const VulkanContext* pContext)
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
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, pContext->vkSurface, &bPresentSupport);

		// if yes, store presentation family queue index!
		if (bPresentSupport)
		{
			m_QueueFamilyIndices.presentFamily = i;
		}

		if (m_QueueFamilyIndices.isComplete())
			break;
	}
}

//-------------------------------------------------------------------------------------------------------------------
bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	// Get count of total number of extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	// gather their information
	m_vecSupportedExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, m_vecSupportedExtensions.data());

	// Compare Required extensions with supported extensions...
	for (int i = 0; i < Helper::g_strDeviceExtensions.size(); ++i)
	{
		bool bExtensionFound = false;

		for (int j = 0; j < extensionCount; ++j)
		{
			// If device supported extensions matches the one we want, good news ... Enumarate them!
			if (strcmp(Helper::g_strDeviceExtensions[i], m_vecSupportedExtensions[j].extensionName) == 0)
			{
				bExtensionFound = true;

				std::string msg = std::string(Helper::g_strDeviceExtensions[i]) + " device extension found!";
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

