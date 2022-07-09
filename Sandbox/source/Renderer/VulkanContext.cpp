#include "sandboxPCH.h"
#include "VulkanContext.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanContext::VulkanContext()
{
	pWindow = nullptr;

	vkInst = VK_NULL_HANDLE;
	vkSurface = VK_NULL_HANDLE;
	vkPhysicalDevice = VK_NULL_HANDLE;
	vkDevice = VK_NULL_HANDLE;

	vkSwapchain = VK_NULL_HANDLE;

	vkQueueGraphics = VK_NULL_HANDLE;
	vkQueuePresent = VK_NULL_HANDLE;

	vkForwardRenderingPipeline = VK_NULL_HANDLE;
	vkForwardRenderingPipelineLayout = VK_NULL_HANDLE;
	vkForwardRenderingRenderPass = VK_NULL_HANDLE;

	vkListFramebuffers.clear();
}

//---------------------------------------------------------------------------------------------------------------------
VulkanContext::~VulkanContext()
{
	pWindow = nullptr;

	vkInst = VK_NULL_HANDLE;
	vkSurface = VK_NULL_HANDLE;
	vkPhysicalDevice = VK_NULL_HANDLE;
	vkDevice = VK_NULL_HANDLE;

	vkSwapchain = VK_NULL_HANDLE;

	vkQueueGraphics = VK_NULL_HANDLE;
	vkQueuePresent = VK_NULL_HANDLE;

	vkForwardRenderingPipeline = VK_NULL_HANDLE;
	vkForwardRenderingPipelineLayout = VK_NULL_HANDLE;
	vkForwardRenderingRenderPass = VK_NULL_HANDLE;

	vkListFramebuffers.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
//--- Create Image View
bool VulkanContext::CreateImageView2D(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView) const
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.format = format;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(vkDevice, &createInfo, nullptr, imageView);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanContext::CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memFlags, VkBuffer* outBuffer, VkDeviceMemory* outMemory) const
{
	// Buffer creation info!
	VkBufferCreateInfo vbInfo = {};
	vbInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vbInfo.size = bufferSize;
	vbInfo.usage = usageFlags;
	vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(vkDevice, &vbInfo, nullptr, outBuffer));

	// Buffer's memory requirements!
	VkMemoryRequirements memReq = {};
	vkGetBufferMemoryRequirements(vkDevice, *outBuffer, &memReq);

	// Allocate memory to buffer!
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(memReq.memoryTypeBits, memFlags);

	VK_CHECK(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, outMemory));

	// Bind memory to given Vertex buffer
	VK_CHECK(vkBindBufferMemory(vkDevice, *outBuffer, *outMemory, 0));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanContext::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize) const
{
	VkCommandBuffer cmdBuffer = BeginCommandBuffer();

	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	CHECK(EndAndSubmitCommandBuffer(cmdBuffer));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
VkCommandBuffer VulkanContext::BeginCommandBuffer() const
{
	// Command buffer to hold transfer command
	VkCommandBuffer commandBuffer;

	// Command buffer details
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vkGraphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	vkAllocateCommandBuffers(vkDevice, &allocInfo, &commandBuffer);

	// Information to begin the command buffer record!
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// We are only using the command buffer once, so set for one time submit!

	// Begin recording transfer commands
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanContext::EndAndSubmitCommandBuffer(VkCommandBuffer commandBuffer) const
{
	// End Commands!
	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Submit transfer command to transfer queue (which is same as Graphics Queue) & wait until it finishes!
	VK_CHECK(vkQueueSubmit(vkQueueGraphics, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(vkQueueGraphics));

	// Free temporary command buffer back to pool!
	vkFreeCommandBuffers(vkDevice, vkGraphicsCommandPool, 1, &commandBuffer);

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
//--- Find suitable memory type based on allowed type & property flags
uint32_t VulkanContext::FindMemoryTypeIndex(uint32_t allowedTypeIndex, VkMemoryPropertyFlags props) const
{
	for (uint32_t i = 0; i < vkDeviceMemoryProps.memoryTypeCount; i++)
	{
		if ((allowedTypeIndex & (1 << i))												// Index of memory type must match corresponding bit in allowed types!
			&& (vkDeviceMemoryProps.memoryTypes[i].propertyFlags & props) == props)		// Desired property bit flags are part of the memory type's property flags!
		{
			// This memory type is valid, so return index!
			return i;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------
//--- Create Shader Module
VkShaderModule VulkanContext::CreateShaderModule(const std::string& fileName) const
{
	// start reading at the end & in binary mode.
	// Advantage of reading file from the end is we can use read position to determine
	// size of the file & allocate buffer accordingly!
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		LOG_ERROR("Failed to open Shader file!");

	// get the file size & allocate buffer memory!
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// now seek back to the beginning of the file & read all bytes at once!
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// close the file!
	file.close();

	// Create Shader Module
	VkShaderModuleCreateInfo shaderModuleInfo;
	shaderModuleInfo.codeSize = buffer.size();
	shaderModuleInfo.flags = 0;
	shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
	shaderModuleInfo.pNext = nullptr;
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	VkShaderModule shaderModule;
	std::string shaderModuleName = fileName;

	vkCreateShaderModule(vkDevice, &shaderModuleInfo, nullptr, &shaderModule);

	return shaderModule;
}