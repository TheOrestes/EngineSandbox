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
VkFormat VulkanContext::ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags) const
{
	for (VkFormat format : formats)
	{
		// Get properties for given formats on this device
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice, format, &props);
		
		// depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}

	LOG_ERROR("Failed to find matching format!");
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanContext::CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, 
									VkMemoryPropertyFlags memoryPropertyFlags, VkImage* pImage, VkDeviceMemory* pDeviceMemory) const
{
	// Image creation info!
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.usage = usageFlags;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create Image!
	VK_CHECK(vkCreateImage(vkDevice, &imageInfo, nullptr, pImage));

	// Get memory requirements for the image...
	VkMemoryRequirements imgMemReqs;
	vkGetImageMemoryRequirements(vkDevice, *pImage, &imgMemReqs);

	// Allocate memory using requirements & user defined properties...
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = imgMemReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(imgMemReqs.memoryTypeBits, memoryPropertyFlags);

	VK_CHECK(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, pDeviceMemory));

	// Connect memory to the image!
	VK_CHECK(vkBindImageMemory(vkDevice, *pImage, *pDeviceMemory, 0));

	return true;
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
bool VulkanContext::CopyImageBuffer(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const
{
	// Create buffer
	VkCommandBuffer transferCommandBuffer = BeginCommandBuffer();

	VkBufferImageCopy imgRegion = {};
	imgRegion.bufferOffset = 0;
	imgRegion.bufferRowLength = 0;
	imgRegion.bufferImageHeight = 0;
	imgRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgRegion.imageSubresource.mipLevel = 0;
	imgRegion.imageSubresource.baseArrayLayer = 0;
	imgRegion.imageSubresource.layerCount = 1;
	imgRegion.imageOffset = { 0,0,0 };
	imgRegion.imageExtent = { width, height, 1 };

	// copy buffer to given image!
	vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgRegion);

	EndAndSubmitCommandBuffer(transferCommandBuffer);

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanContext::TransitionImageLayout(VkImage srcImage, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer cmdBuffer) const
{
	VkCommandBuffer commandBuffer;

	if (cmdBuffer != VK_NULL_HANDLE)
		commandBuffer = cmdBuffer;
	else
		commandBuffer = BeginCommandBuffer();

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;									// Layout to transition from
	imageMemoryBarrier.newLayout = newLayout;									// Layout to transition to
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// Queue family to transition from
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// Queue family to transition to
	imageMemoryBarrier.image = srcImage;										// Image being accessed & modified as a part of barrier
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Aspect of image being altered
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;						// First mip level to start alteration on
	imageMemoryBarrier.subresourceRange.levelCount = 1;							// Number of mip levels to alter starting from base mip level
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;						// First layer of start alterations on
	imageMemoryBarrier.subresourceRange.layerCount = 1;							// Number of layers to alter starting from base array layer

	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	// Credit = Sascha Willems : VulkanTools.cpp!
	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	// ? Added srcStage logic too, only where it's required! 
	switch (oldLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
		{
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
		{
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		{
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		{
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		{
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		{
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		}

		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		{
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		{
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		{
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		}

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		{
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			break;
		}

		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	vkCmdPipelineBarrier(	commandBuffer,
							srcStage, dstStage,		// Pipeline stages (match to src & dest AcccessMask)
							0,						// Dependency flags
							0, nullptr,				// Memory barrier count + data
							0, nullptr,				// Buffer memory barrier count + data
							1, &imageMemoryBarrier);// image memmory barrier count + data

	EndAndSubmitCommandBuffer(commandBuffer);

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