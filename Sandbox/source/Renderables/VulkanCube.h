#pragma once

#include "glm/glm.hpp"
#include "Renderer/Utility.h"
#include "VulkanMesh.h"

class VulkanContext;
class VulkanMaterial;

//---------------------------------------------------------------------------------------------------------------------
struct UniformData
{
	UniformData()
	{
		matWorld = glm::mat4(1);
		matView = glm::mat4(1);
		matProjection = glm::mat4(1);
	}

	// Transformation data...
	alignas(64) glm::mat4 matWorld;
	alignas(64) glm::mat4 matView;
	alignas(64) glm::mat4 matProjection;

	// Material data...
	alignas(16)	glm::vec4	albedoColor;
	alignas(16) glm::vec4	emissionColor;
	alignas(16) glm::vec3	hasTextureAEN;
	alignas(16) glm::vec3	hasTextureRMO;
	alignas(4)	float		occlusion;
	alignas(4)	float		roughness;
	alignas(4)	float		metalness;
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
	//VulkanCube(const glm::vec3& color);

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
	VulkanMaterial*						m_pMaterial;
	std::vector<Helper::VertexPT>		m_ListVertices;
	std::vector<uint32_t>				m_ListIndices;

public:
	// Transformations!
	glm::vec3							m_vecPosition;
	glm::vec3							m_vecRotationAxis;
	glm::vec3							m_vecScale;
	float								m_fRotation;
};

