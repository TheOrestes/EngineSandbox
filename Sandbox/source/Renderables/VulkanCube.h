#pragma once

#include "glm/glm.hpp"
#include "Renderer/Utility.h"
#include "VulkanMesh.h"

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
class VulkanCube
{
public:
	VulkanCube();
	VulkanCube(const glm::vec3& color);

	~VulkanCube();

	bool								InitCube(const VulkanContext* pContext);
	void								Render(const VulkanContext* pContext, uint32_t index);
	void								Update(const VulkanContext* pContext, float dt);
	void								UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex);
	void								Cleanup(VulkanContext* pContext);
	void								CleanupOnWindowsResize(VulkanContext* pContext);

private:
	bool								SetupDescriptors(const VulkanContext* pContext);
	bool								CreateDescriptorPool(const VulkanContext* pContext);
	bool								CreateDescriptorSetLayout(const VulkanContext* pContext);
	bool								CreateDescriptorSets(const VulkanContext* pContext);

public:
	UniformDataBuffer*					m_pShaderDataBuffer;
	VkDescriptorPool					m_vkDescriptorPool;
	VkDescriptorSetLayout				m_vkDescriptorSetLayout;
	std::vector<VkDescriptorSet>		m_ListDescriptorSets;

private:
	VulkanMesh*							m_pMesh;
	std::vector<Helper::VertexPC>		m_ListVertices;
	std::vector<uint32_t>				m_ListIndices;

public:
	// Transformations!
	glm::vec3							m_vecPosition;
	glm::vec3							m_vecRotationAxis;
	glm::vec3							m_vecScale;
	float								m_fRotation;
};

