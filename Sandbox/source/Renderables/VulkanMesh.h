#pragma once

#include "vulkan/vulkan.h"
#include "Renderer/Utility.h"

class VulkanContext;

class VulkanMesh
{
public:
	VulkanMesh() : m_uiVertexCount(0), m_vkVertexBuffer(VK_NULL_HANDLE) {}
	VulkanMesh(const VulkanContext* pRC, const std::vector<Helper::VertexPC>& vertices);

	void		Cleanup(VulkanContext* pContext);

	~VulkanMesh();

public:
	uint32_t		m_uiVertexCount;
	VkBuffer		m_vkVertexBuffer;
	VkDeviceMemory	m_vkVertexBufferMemory;

private:
	void		CreateVertexBuffer(const VulkanContext* pContext, const std::vector<Helper::VertexPC>& vertices);
};

