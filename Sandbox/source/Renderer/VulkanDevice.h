#pragma once

#include "vulkan/vulkan.hpp"

class VulkanDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

	void				CreatePhysicalDevice();
	void				CreateLogicalDevice();

private:
	VkPhysicalDevice	m_vkPhysicalDevice;
	VkDevice			m_vkLogicalDevice;
};

