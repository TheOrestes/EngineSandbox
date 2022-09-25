#pragma once

#include "vulkan/vulkan.h"
#include "Renderer/Utility.h"

class VulkanContext;

//---------------------------------------------------------------------------------------------------------------------
class VulkanMesh
{
public:
	VulkanMesh() : 
		m_uiVertexCount(0), 
		m_uiIndexCount(0),
		m_vkVertexBuffer(VK_NULL_HANDLE), 
		m_vkIndexBuffer(VK_NULL_HANDLE),
		m_vkVertexBufferMemory(VK_NULL_HANDLE),
		m_vkIndexBufferMemory(VK_NULL_HANDLE) {}

	VulkanMesh(const VulkanContext* pRC, const std::vector<Helper::VertexPNTBT>& vertices, const std::vector<uint32_t>& indices);

	void							Cleanup(VulkanContext* pContext);

	~VulkanMesh();

public:
	uint32_t						m_uiVertexCount;
	uint32_t						m_uiIndexCount;

	VkBuffer						m_vkVertexBuffer;
	VkDeviceMemory					m_vkVertexBufferMemory;

	VkBuffer						m_vkIndexBuffer;
	VkDeviceMemory					m_vkIndexBufferMemory;

private:
	void							CreateVertexBuffer(const VulkanContext* pContext, const std::vector<Helper::VertexPNTBT>& vertices);
	void							CreateIndexBuffer(const VulkanContext* pContext, const std::vector<uint32_t>& indices);
};

