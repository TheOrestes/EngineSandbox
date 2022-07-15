#include "sandboxPCH.h"
#include "VulkanMesh.h"
#include "Renderer/VulkanContext.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanMesh::~VulkanMesh()
{
	SAFE_DELETE(m_pShaderDataBuffer);
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanMesh::VulkanMesh(const VulkanContext* pContext, const std::vector<Helper::VertexPC>& vertices, const std::vector<uint32_t>& indices)
{
	m_uiVertexCount = vertices.size();
	m_uiIndexCount = indices.size();

	CreateVertexBuffer(pContext, vertices);
	CreateIndexBuffer(pContext, indices);

	m_pShaderDataBuffer = new UniformDataBuffer();
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::Cleanup(VulkanContext* pContext)
{
	m_pShaderDataBuffer->Cleanup(pContext);

	vkDestroyDescriptorPool(pContext->vkDevice, m_vkDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(pContext->vkDevice, m_vkDescriptorSetLayout, nullptr);

	vkDestroyBuffer(pContext->vkDevice, m_vkIndexBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, m_vkIndexBufferMemory, nullptr);

	vkDestroyBuffer(pContext->vkDevice, m_vkVertexBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, m_vkVertexBufferMemory, nullptr);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::CreateVertexBuffer(const VulkanContext* pContext, const std::vector<Helper::VertexPC>& vertices)
{
	// Get the size of buffer needed for vertices
	VkDeviceSize bufferSize = m_uiVertexCount * sizeof(Helper::VertexPC);

	// Temp buffer to "stage" vertex data before transferring to GPU memory!
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create buffer & allocate memory to it!
	pContext->CreateBuffer(	bufferSize,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							&stagingBuffer, &stagingBufferMemory);

	// Map memory to Vertex buffer
	void* data;
	vkMapMemory(pContext->vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(pContext->vkDevice, stagingBufferMemory);

	// Now, Create buffer with TRANSFER_DST_BIT to make as recipient of data (also VERTEX_BUFFER_BIT)
	// this time, buffer memory is set to DEVICE_LOCAL which means, it's on the GPU. 
	pContext->CreateBuffer(	bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							&m_vkVertexBuffer,
							&m_vkVertexBufferMemory);

	// Copy staging buffer data to vertex buffer on GPUP using Command buffer!
	pContext->CopyBuffer(stagingBuffer, m_vkVertexBuffer, bufferSize);

	// Clean up staging buffers
	vkDestroyBuffer(pContext->vkDevice, stagingBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, stagingBufferMemory, nullptr);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::CreateIndexBuffer(const VulkanContext* pContext, const std::vector<uint32_t>& indices)
{
	// Get size of buffer needed for indices
	VkDeviceSize bufferSize = m_uiIndexCount * sizeof(uint32_t);

	// Temporary buffer to "stage" index data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	pContext->CreateBuffer(	bufferSize,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							&stagingBuffer,
							&stagingBufferMemory);

	// Map memory to Index buffer
	void* data;
	vkMapMemory(pContext->vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(pContext->vkDevice, stagingBufferMemory);

	// Create buffer for index data on GPU access only area
	pContext->CreateBuffer(	bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							&m_vkIndexBuffer,
							&m_vkIndexBufferMemory);

	// Copy from staging buffer to GPU access buffer
	pContext->CopyBuffer(stagingBuffer, m_vkIndexBuffer, bufferSize);

	// Clean up staging buffers
	vkDestroyBuffer(pContext->vkDevice, stagingBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, stagingBufferMemory, nullptr);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::Update(const VulkanContext* pContext, float dt)
{
	static float fCurrentAngle = 0.0f;  
	fCurrentAngle += dt * 0.1f;
	if (fCurrentAngle > 360.0f) { fCurrentAngle = 0.0f; }

	m_pShaderDataBuffer->shaderData.matWorld = glm::mat4(1);
	m_pShaderDataBuffer->shaderData.matWorld = glm::rotate(m_pShaderDataBuffer->shaderData.matWorld, fCurrentAngle, glm::vec3(0, 1, 0));

	float aspect = (float)pContext->vkSwapchainExtent.width / (float)pContext->vkSwapchainExtent.height;
	m_pShaderDataBuffer->shaderData.matProjection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
	m_pShaderDataBuffer->shaderData.matProjection[1][1] *= -1.0f;

	m_pShaderDataBuffer->shaderData.matView = glm::lookAt(glm::vec3(3.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex)
{
	void* data;
	vkMapMemory(pContext->vkDevice, m_pShaderDataBuffer->listDeviceMemory[imageIndex], 0, sizeof(UniformData), 0, &data);
	memcpy(data, &(m_pShaderDataBuffer->shaderData), sizeof(UniformData));
	vkUnmapMemory(pContext->vkDevice, m_pShaderDataBuffer->listDeviceMemory[imageIndex]);
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMesh::SetupDescriptors(const VulkanContext* pContext)
{
	m_pShaderDataBuffer->CreateUniformDataBuffers(pContext);

	// Descriptor Pool
	CHECK(CreateDescriptorPool(pContext));

	// Descriptor Set Layout
	CHECK(CreateDescriptorSetLayout(pContext));

	// Descriptor Sets
	CHECK(CreateDescriptorSets(pContext));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMesh::CreateDescriptorPool(const VulkanContext* pContext)
{
	std::array<VkDescriptorPoolSize, 1> arrDescriptorPoolSize = {};

	//-- Uniform buffers
	arrDescriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	arrDescriptorPoolSize[0].descriptorCount = pContext->uiNumSwapchainImages;

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = pContext->uiNumSwapchainImages;
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(arrDescriptorPoolSize.size());
	poolCreateInfo.pPoolSizes = arrDescriptorPoolSize.data();

	// Create Descriptor Pool!
	VK_CHECK(vkCreateDescriptorPool(pContext->vkDevice, &poolCreateInfo, nullptr, &m_vkDescriptorPool));
	
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMesh::CreateDescriptorSetLayout(const VulkanContext* pContext)
{
	std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings;

	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutCreateInfo.pBindings = layoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(pContext->vkDevice, &layoutCreateInfo, nullptr, &m_vkDescriptorSetLayout));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMesh::CreateDescriptorSets(const VulkanContext* pContext)
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
		ubBufferInfo.range = sizeof(UniformData);

		VkWriteDescriptorSet ubWriteSet = {};
		ubWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubWriteSet.descriptorCount = 1;
		ubWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubWriteSet.dstArrayElement = 0;
		ubWriteSet.dstBinding = 0;
		ubWriteSet.dstSet = m_ListDescriptorSets[i];
		ubWriteSet.pBufferInfo = &ubBufferInfo;

		// List of all Descriptor set writes!
		std::vector<VkWriteDescriptorSet> listWriteSets = { ubWriteSet };

		// Update the descriptor sets with buffers/binding info
		vkUpdateDescriptorSets(pContext->vkDevice, static_cast<uint32_t>(listWriteSets.size()), listWriteSets.data(), 0, nullptr);
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
void UniformDataBuffer::CreateUniformDataBuffers(const VulkanContext* pContext)
{
	VkDeviceSize bufferSize = sizeof(UniformData);

	// One uniform buffer for each swapchain (or command buffer)
	listBuffers.resize(pContext->uiNumSwapchainImages);
	listDeviceMemory.resize(pContext->uiNumSwapchainImages);

	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages ; i++)
	{
		pContext->CreateBuffer(	bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								&listBuffers[i], &listDeviceMemory[i]);
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void UniformDataBuffer::Cleanup(const VulkanContext* pContext)
{
	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages ; i++)
	{
		vkDestroyBuffer(pContext->vkDevice, listBuffers[i], nullptr);
		vkFreeMemory(pContext->vkDevice, listDeviceMemory[i], nullptr);
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void UniformDataBuffer::CleanupOnWindowsResize(const VulkanContext* pContext)
{
}
