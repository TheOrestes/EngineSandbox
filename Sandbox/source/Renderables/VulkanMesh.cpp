#include "sandboxPCH.h"
#include "VulkanMesh.h"
#include "Renderer/VulkanContext.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanMesh::~VulkanMesh()
{
	
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanMesh::VulkanMesh(const VulkanContext* pContext, const std::vector<Helper::VertexPT>& vertices, const std::vector<uint32_t>& indices)
{
	m_uiVertexCount = vertices.size();
	m_uiIndexCount = indices.size();

	CreateVertexBuffer(pContext, vertices);
	CreateIndexBuffer(pContext, indices);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::Cleanup(VulkanContext* pContext)
{
	vkDestroyBuffer(pContext->vkDevice, m_vkIndexBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, m_vkIndexBufferMemory, nullptr);

	vkDestroyBuffer(pContext->vkDevice, m_vkVertexBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, m_vkVertexBufferMemory, nullptr);
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMesh::CreateVertexBuffer(const VulkanContext* pContext, const std::vector<Helper::VertexPT>& vertices)
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





