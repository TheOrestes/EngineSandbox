#pragma once

#include "vulkan/vulkan.h"
#include "Renderer/Utility.h"

class VulkanContext;

//---------------------------------------------------------------------------------------------------------------------
struct UniformData
{
	alignas(64) glm::mat4 matWorld;
	alignas(64) glm::mat4 matView;
	alignas(64) glm::mat4 matProjection;
};

//---------------------------------------------------------------------------------------------------------------------
struct UniformDataBuffer
{
	UniformDataBuffer()
	{
		listBuffers.clear();
		listDeviceMemory.clear();
	}

	void						CreateUniformDataBuffers(const VulkanContext* pContext);
	void						Cleanup(const VulkanContext* pContext);
	void						CleanupOnWindowsResize(const VulkanContext* pContext);

	UniformData					shaderData;

	std::vector<VkBuffer>		listBuffers;
	std::vector<VkDeviceMemory>	listDeviceMemory;
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanMesh
{
public:
	VulkanMesh() : m_uiVertexCount(0), m_vkVertexBuffer(VK_NULL_HANDLE), m_vkIndexBuffer(VK_NULL_HANDLE) {}
	VulkanMesh(const VulkanContext* pRC, const std::vector<Helper::VertexPC>& vertices, const std::vector<uint32_t>& indices);

	bool							SetupDescriptors(const VulkanContext* pContext);
	void							Update(const VulkanContext* pContext, float dt);
	void							UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex);
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
	void							CreateVertexBuffer(const VulkanContext* pContext, const std::vector<Helper::VertexPC>& vertices);
	void							CreateIndexBuffer(const VulkanContext* pContext, const std::vector<uint32_t>& indices);

	bool							CreateDescriptorPool(const VulkanContext* pContext);
	bool							CreateDescriptorSetLayout(const VulkanContext* pContext);
	bool							CreateDescriptorSets(const VulkanContext* pContext);

	UniformDataBuffer*				m_pShaderDataBuffer;

public:
	VkDescriptorPool				m_vkDescriptorPool;
	VkDescriptorSetLayout			m_vkDescriptorSetLayout;
	std::vector<VkDescriptorSet>	m_ListDescriptorSets;
};

