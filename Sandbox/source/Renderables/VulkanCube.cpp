#include "sandboxPCH.h"
#include "VulkanCube.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanMaterial.h"
#include "Renderer/VulkanTexture.h"
#include "Renderer/Utility.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanCube::VulkanCube()
{
	// Vertex Data!
	m_ListVertices.resize(8);

	m_ListVertices[0] = Helper::VertexPNTBT(glm::vec3(-1, -1, 1),   glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f));
	m_ListVertices[1] = Helper::VertexPNTBT(glm::vec3(1, -1, 1),    glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f));
	m_ListVertices[2] = Helper::VertexPNTBT(glm::vec3(1, 1, 1),     glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f));
	m_ListVertices[3] = Helper::VertexPNTBT(glm::vec3(-1, 1, 1),    glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f));
	m_ListVertices[4] = Helper::VertexPNTBT(glm::vec3(-1, -1, -1),  glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 1.0f));
	m_ListVertices[5] = Helper::VertexPNTBT(glm::vec3(1, -1, -1),   glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 1.0f));
	m_ListVertices[6] = Helper::VertexPNTBT(glm::vec3(1, 1, -1),    glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0.0f, 0.0f));
	m_ListVertices[7] = Helper::VertexPNTBT(glm::vec3(-1, 1, -1),   glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1.0f, 0.0f));

	// Index Data!
	m_ListIndices.resize(36);

	m_ListIndices[0] = 0;				m_ListIndices[1] = 1;			m_ListIndices[2] = 2;
	m_ListIndices[3] = 2;				m_ListIndices[4] = 3;			m_ListIndices[5] = 0;

	m_ListIndices[6] = 3;				m_ListIndices[7] = 2;			m_ListIndices[8] = 6;
	m_ListIndices[9] = 6;				m_ListIndices[10] = 7;			m_ListIndices[11] = 3;

	m_ListIndices[12] = 7;				m_ListIndices[13] = 6;			m_ListIndices[14] = 5;
	m_ListIndices[15] = 5;				m_ListIndices[16] = 4;			m_ListIndices[17] = 7;

	m_ListIndices[18] = 4;				m_ListIndices[19] = 5;			m_ListIndices[20] = 1;
	m_ListIndices[21] = 1;				m_ListIndices[22] = 0;			m_ListIndices[23] = 4;

	m_ListIndices[24] = 4;				m_ListIndices[25] = 0;			m_ListIndices[26] = 3;
	m_ListIndices[27] = 3;				m_ListIndices[28] = 7;			m_ListIndices[29] = 4;

	m_ListIndices[30] = 1;				m_ListIndices[31] = 5;			m_ListIndices[32] = 6;
	m_ListIndices[33] = 6;				m_ListIndices[34] = 2;			m_ListIndices[35] = 1;

	m_pShaderDataBuffer = nullptr;
	m_pMesh = nullptr;
	m_pMaterial = nullptr;

	m_vecPosition = glm::vec3(0,0,-2);
	m_vecRotationAxis = glm::vec3(0, 1, 0);
	m_vecScale = glm::vec3(1, 1, 1);
	m_fRotation = 0.0f;
}

//---------------------------------------------------------------------------------------------------------------------
//VulkanCube::VulkanCube(const glm::vec3& color)
//{
//	// Vertex Data!
//	m_ListVertices.resize(8);
//
//	m_ListVertices[0] = Helper::VertexPC(glm::vec3(-1, -1, 1),	color);
//	m_ListVertices[1] = Helper::VertexPC(glm::vec3(1, -1, 1),	color);
//	m_ListVertices[2] = Helper::VertexPC(glm::vec3(1, 1, 1),	color);
//	m_ListVertices[3] = Helper::VertexPC(glm::vec3(-1, 1, 1),	color);
//	m_ListVertices[4] = Helper::VertexPC(glm::vec3(-1, -1, -1),	color);
//	m_ListVertices[5] = Helper::VertexPC(glm::vec3(1, -1, -1),	color);
//	m_ListVertices[6] = Helper::VertexPC(glm::vec3(1, 1, -1),	color);
//	m_ListVertices[7] = Helper::VertexPC(glm::vec3(-1, 1, -1),	color);
//
//	// Index Data!
//	m_ListIndices.resize(36);
//
//	m_ListIndices[0] = 0;				m_ListIndices[1] = 1;			m_ListIndices[2] = 2;
//	m_ListIndices[3] = 2;				m_ListIndices[4] = 3;			m_ListIndices[5] = 0;
//
//	m_ListIndices[6] = 3;				m_ListIndices[7] = 2;			m_ListIndices[8] = 6;
//	m_ListIndices[9] = 6;				m_ListIndices[10] = 7;			m_ListIndices[11] = 3;
//
//	m_ListIndices[12] = 7;				m_ListIndices[13] = 6;			m_ListIndices[14] = 5;
//	m_ListIndices[15] = 5;				m_ListIndices[16] = 4;			m_ListIndices[17] = 7;
//
//	m_ListIndices[18] = 4;				m_ListIndices[19] = 5;			m_ListIndices[20] = 1;
//	m_ListIndices[21] = 1;				m_ListIndices[22] = 0;			m_ListIndices[23] = 4;
//
//	m_ListIndices[24] = 4;				m_ListIndices[25] = 0;			m_ListIndices[26] = 3;
//	m_ListIndices[27] = 3;				m_ListIndices[28] = 7;			m_ListIndices[29] = 4;
//
//	m_ListIndices[30] = 1;				m_ListIndices[31] = 5;			m_ListIndices[32] = 6;
//	m_ListIndices[33] = 6;				m_ListIndices[34] = 2;			m_ListIndices[35] = 1;
//
//	m_pShaderDataBuffer = nullptr;
//	m_pMesh = nullptr;
//	m_pMaterial = nullptr;
//
//	m_vecPosition = glm::vec3(0, 0, 4);
//	m_vecRotationAxis = glm::vec3(0, 1, 0);
//	m_vecScale = glm::vec3(1, 1, 1);
//	m_fRotation = 0.0f;
//}

//---------------------------------------------------------------------------------------------------------------------
VulkanCube::~VulkanCube()
{
	SAFE_DELETE(m_pMaterial);
	SAFE_DELETE(m_pMesh);
	SAFE_DELETE(m_pShaderDataBuffer);

	m_ListVertices.clear();
	m_ListIndices.clear();
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanCube::InitCube(const VulkanContext* pContext)
{
	m_pMesh = new VulkanMesh(pContext, m_ListVertices, m_ListIndices);

	m_pMaterial = new VulkanMaterial();
	m_pMaterial->LoadTexture(pContext, "Assets/Textures/Cube/Default.png", TextureType::TEXTURE_ALBEDO);
	
	CHECK(SetupDescriptors(pContext));

    return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanCube::SetupDescriptors(const VulkanContext* pContext)
{
	m_pShaderDataBuffer = new UniformDataBufferCube();
	m_pShaderDataBuffer->CreateUniformDataBuffers(pContext);

	// Set default material info!
	m_pShaderDataBuffer->shaderData.albedoColor = glm::vec4(1);
	m_pShaderDataBuffer->shaderData.emissionColor = glm::vec4(1);
	m_pShaderDataBuffer->shaderData.hasTextureAEN = glm::vec3(1, 0, 0);
	m_pShaderDataBuffer->shaderData.hasTextureRMO = glm::vec3(0);
	m_pShaderDataBuffer->shaderData.metalness = 0.0f;
	m_pShaderDataBuffer->shaderData.occlusion = 1.0f;
	m_pShaderDataBuffer->shaderData.roughness = 1.0f;

	// Descriptor Pool
	CHECK(CreateDescriptorPool(pContext));

	// Descriptor Set Layout
	CHECK(CreateDescriptorSetLayout(pContext));

	// Descriptor Sets
	CHECK(CreateDescriptorSets(pContext));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanCube::CreateDescriptorPool(const VulkanContext* pContext)
{
	std::array<VkDescriptorPoolSize, 2> arrDescriptorPoolSize = {};

	//-- Uniform buffers
	arrDescriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	arrDescriptorPoolSize[0].descriptorCount = pContext->uiNumSwapchainImages;

	//-- Texture samplers
	arrDescriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	arrDescriptorPoolSize[1].descriptorCount = m_pMaterial->m_uiNumTextures;

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = pContext->uiNumSwapchainImages + m_pMaterial->m_uiNumTextures;
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(arrDescriptorPoolSize.size());
	poolCreateInfo.pPoolSizes = arrDescriptorPoolSize.data();

	// Create Descriptor Pool!
	VK_CHECK(vkCreateDescriptorPool(pContext->vkDevice, &poolCreateInfo, nullptr, &m_vkDescriptorPool));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanCube::CreateDescriptorSetLayout(const VulkanContext* pContext)
{
	std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings;

	// Uniform buffer
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[0].pImmutableSamplers = nullptr;

	// Albedo texture
	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[1].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutCreateInfo.pBindings = layoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(pContext->vkDevice, &layoutCreateInfo, nullptr, &m_vkDescriptorSetLayout));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanCube::CreateDescriptorSets(const VulkanContext* pContext)
{
	//-- Descriptor Sets!
	m_ListDescriptorSets.resize(pContext->uiNumSwapchainImages);

	// create copy of descriptor set layout for each swap chain image!
	std::vector<VkDescriptorSetLayout> listSetLayouts(pContext->uiNumSwapchainImages, m_vkDescriptorSetLayout);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = m_vkDescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32_t>(listSetLayouts.size());
	setAllocInfo.pSetLayouts = listSetLayouts.data();

	VK_CHECK(vkAllocateDescriptorSets(pContext->vkDevice, &setAllocInfo, m_ListDescriptorSets.data()));

	//-- Update all the descriptor set bindings!
	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages; i++)
	{
		//-- Uniform buffer
		VkDescriptorBufferInfo ubBufferInfo = {};
		ubBufferInfo.buffer = m_pShaderDataBuffer->listBuffers[i];
		ubBufferInfo.offset = 0;
		ubBufferInfo.range = sizeof(UniformDataCube);

		VkWriteDescriptorSet ubWriteSet = {};
		ubWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubWriteSet.descriptorCount = 1;
		ubWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubWriteSet.dstArrayElement = 0;
		ubWriteSet.dstBinding = 0;
		ubWriteSet.dstSet = m_ListDescriptorSets[i];
		ubWriteSet.pBufferInfo = &ubBufferInfo;

		//-- Albedo Texture
		VkDescriptorImageInfo albedoImageInfo = {};
		albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoImageInfo.imageView = m_pMaterial->m_pTextureAlbedo->getVkImageView();
		albedoImageInfo.sampler = m_pMaterial->m_pTextureAlbedo->getVkSampler();

		VkWriteDescriptorSet albedoWriteSet = {};
		albedoWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		albedoWriteSet.dstSet = m_ListDescriptorSets[i];
		albedoWriteSet.dstBinding = 1;
		albedoWriteSet.dstArrayElement = 0;
		albedoWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		albedoWriteSet.descriptorCount = 1;
		albedoWriteSet.pImageInfo = &albedoImageInfo;

		// List of all Descriptor set writes!
		std::vector<VkWriteDescriptorSet> listWriteSets = { ubWriteSet, albedoWriteSet };

		// Update the descriptor sets with buffers/binding info
		vkUpdateDescriptorSets(pContext->vkDevice, static_cast<uint32_t>(listWriteSets.size()), listWriteSets.data(), 0, nullptr);
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::Render(const VulkanContext* pContext, uint32_t index)
{
	std::vector<VkBuffer> vertexBuffers = { m_pMesh->m_vkVertexBuffer };
	VkBuffer indexBuffer = m_pMesh->m_vkIndexBuffer;
	std::vector<VkDeviceSize> offsets = { 0 };

	// Bind VB & IB
	vkCmdBindVertexBuffers(pContext->vkListGraphicsCommandBuffers[index], 0, 1, vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(pContext->vkListGraphicsCommandBuffers[index], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	// Bind Descriptor Sets
	vkCmdBindDescriptorSets(pContext->vkListGraphicsCommandBuffers[index],
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pContext->vkForwardRenderingPipelineLayout,
							0,
							1,
							&(m_ListDescriptorSets[index]),
							0, nullptr);

	// Execute
	vkCmdDrawIndexed(pContext->vkListGraphicsCommandBuffers[index], m_pMesh->m_uiIndexCount, 1, 0, 0, 0);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::Update(const VulkanContext* pContext, float dt)
{
	static float fCurrentAngle = 0.0f;
	fCurrentAngle += dt * 0.05f;
	if (fCurrentAngle > 360.0f) { fCurrentAngle = 0.0f; }

	m_pShaderDataBuffer->shaderData.matWorld = glm::mat4(1);
	m_pShaderDataBuffer->shaderData.matWorld = glm::translate(m_pShaderDataBuffer->shaderData.matWorld, m_vecPosition);
	m_pShaderDataBuffer->shaderData.matWorld = glm::rotate(m_pShaderDataBuffer->shaderData.matWorld, fCurrentAngle, m_vecRotationAxis);
	m_pShaderDataBuffer->shaderData.matWorld = glm::scale(m_pShaderDataBuffer->shaderData.matWorld, m_vecScale);

	float aspect = (float)pContext->vkSwapchainExtent.width / (float)pContext->vkSwapchainExtent.height;
	m_pShaderDataBuffer->shaderData.matProjection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
	m_pShaderDataBuffer->shaderData.matProjection[1][1] *= -1.0f;

	m_pShaderDataBuffer->shaderData.matView = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex)
{
	void* data;
	vkMapMemory(pContext->vkDevice, m_pShaderDataBuffer->listDeviceMemory[imageIndex], 0, sizeof(UniformDataCube), 0, &data);
	memcpy(data, &(m_pShaderDataBuffer->shaderData), sizeof(UniformDataCube));
	vkUnmapMemory(pContext->vkDevice, m_pShaderDataBuffer->listDeviceMemory[imageIndex]);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::Cleanup(VulkanContext* pContext)
{
	m_pMesh->Cleanup(pContext);
	m_pShaderDataBuffer->Cleanup(pContext);

	vkDestroyDescriptorPool(pContext->vkDevice, m_vkDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(pContext->vkDevice, m_vkDescriptorSetLayout, nullptr);

	m_ListVertices.clear();
	m_ListIndices.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::CleanupOnWindowsResize(VulkanContext* pContext)
{
}

//-----------------------------------------------------------------------------------------------------------------------
void UniformDataBufferCube::CreateUniformDataBuffers(const VulkanContext* pContext)
{
	VkDeviceSize bufferSize = sizeof(UniformDataCube);

	// One uniform buffer for each swapchain (or command buffer)
	listBuffers.resize(pContext->uiNumSwapchainImages);
	listDeviceMemory.resize(pContext->uiNumSwapchainImages);

	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages; i++)
	{
		pContext->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&listBuffers[i], &listDeviceMemory[i]);
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void UniformDataBufferCube::Cleanup(const VulkanContext* pContext)
{
	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages; i++)
	{
		vkDestroyBuffer(pContext->vkDevice, listBuffers[i], nullptr);
		vkFreeMemory(pContext->vkDevice, listDeviceMemory[i], nullptr);
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void UniformDataBufferCube::CleanupOnWindowsResize(const VulkanContext* pContext)
{
}
