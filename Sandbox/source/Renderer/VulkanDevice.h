#pragma once

#include "vulkan/vulkan.h"

struct VulkanContext;
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
	VulkanDevice(const VulkanContext* pRC);
	~VulkanDevice();

	bool								SetupDevice(VulkanContext* pRC);
	bool								CreateCommandPool(VulkanContext* pRC);
	bool								CreateCommandBuffers(VulkanContext* pRC);
	void								HandleWindowsResize(VulkanContext* pRC);
	void								Cleanup(VulkanContext* pRC);
	void								CleanupOnWindowsResize(VulkanContext* pRC);

private:
	bool								AcquirePhysicalDevice(VulkanContext* pRC);
	bool								CreateLogicalDevice(VulkanContext* pRC);
	void								FetchQueueFamilies(VkPhysicalDevice physicalDevice, const VulkanContext* pRC);
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

