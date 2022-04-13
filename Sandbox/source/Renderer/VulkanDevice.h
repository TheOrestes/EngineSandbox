#pragma once

#include "vulkan/vulkan.hpp"

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

	bool isComplete() { return graphicsFamily.has_value(); /*&& presentFamily.has_value()*/ }
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanDevice
{
public:
	VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
	~VulkanDevice();

	bool								AcquirePhysicalDevice();
	bool								CreateLogicalDevice();

private:
	void								FetchQueueFamilies(VkPhysicalDevice physicalDevice);
	bool								CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

private:
	VkPhysicalDevice					m_vkPhysicalDevice;
	VkPhysicalDeviceProperties			m_vkDeviceProps;
	VkPhysicalDeviceFeatures			m_vkDeviceFeaturesAvailable;
	VkPhysicalDeviceFeatures			m_vkDeviceFeaturesEnabled;
	VkPhysicalDeviceMemoryProperties	m_vkDeviceMemoryProps;
	std::vector<VkExtensionProperties>	m_vecSupportedExtensions;

	VkDevice							m_vkLogicalDevice;
	VkInstance							m_vkInstance;
	VkSurfaceKHR						m_vkSurfaceKHR;

	QueueFamilyIndices					m_QueueFamilyIndices;
};

