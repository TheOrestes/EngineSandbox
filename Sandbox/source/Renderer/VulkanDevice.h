#pragma once

#include "vulkan/vulkan.h"

struct RenderContext;
class VulkanSwapchain;

//---------------------------------------------------------------------------------------------------------------------
struct QueueFamilyIndices
{
	QueueFamilyIndices()
	{
		graphicsFamily.reset();
		presentFamily.reset();
	}

	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanDevice
{
public:
	VulkanDevice(const RenderContext* pRC);
	~VulkanDevice();

	bool								SetupDevice(RenderContext* pRC);
	void								Destroy();

private:
	bool								AcquirePhysicalDevice(RenderContext* pRC);
	bool								CreateLogicalDevice(RenderContext* pRC);
	void								FetchQueueFamilies(VkPhysicalDevice physicalDevice, const RenderContext* pRC);
	bool								CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

private:
	
	VkPhysicalDeviceProperties			m_vkDeviceProps;
	VkPhysicalDeviceFeatures			m_vkDeviceFeaturesAvailable;
	VkPhysicalDeviceFeatures			m_vkDeviceFeaturesEnabled;
	VkPhysicalDeviceMemoryProperties	m_vkDeviceMemoryProps;
	std::vector<VkExtensionProperties>	m_vecSupportedExtensions;
	
public:
	VulkanSwapchain*					m_pSwapchain;
	QueueFamilyIndices					m_QueueFamilyIndices;
};

