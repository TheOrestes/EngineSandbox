#include "sandboxPCH.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "Core/Core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanTexture::VulkanTexture()
{
}

//---------------------------------------------------------------------------------------------------------------------
VulkanTexture::~VulkanTexture()
{
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanTexture::CreateTexture(const VulkanContext* pContext, const std::string& filename, VkFormat format)
{
	CHECK(CreateImage(pContext, filename, format));
	CHECK(pContext->CreateImageView2D(m_Image.image, format, VK_IMAGE_ASPECT_COLOR_BIT, &(m_Image.imageView)));

	// Create Sampler
	CHECK(CreateTextureSampler(pContext));

	LOG_DEBUG("Created Vulkan Texture for {0}", filename);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanTexture::Cleanup(const VulkanContext* pContext)
{
	vkDestroySampler(pContext->vkDevice, m_vkTextureSampler, nullptr);

	vkDestroyImageView(pContext->vkDevice, m_Image.imageView, nullptr);
	vkDestroyImage(pContext->vkDevice, m_Image.image, nullptr);
	vkFreeMemory(pContext->vkDevice, m_Image.deviceMemory, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanTexture::CleanupOnWindowResize(const VulkanContext* pContext)
{
}

//---------------------------------------------------------------------------------------------------------------------
unsigned char* VulkanTexture::LoadImageData(const VulkanContext* pContext, const std::string& filename)
{
	// Number of channels in image
	int channels = 0;

	std::string fileLoc = "Assets//Textures//" + filename;

	// Load pixel data for an image
	unsigned char* imageData = stbi_load(fileLoc.c_str(), &m_iTextureWidth, &m_iTextureHeight, &m_iTextureChannels, STBI_rgb_alpha);
	if (!imageData)
	{
		LOG_ERROR(("Failed to load a Texture file! (" + filename + ")").c_str());
	}

	// Calculate image size using given data
	m_vkTextureDeviceSize = m_iTextureWidth * m_iTextureHeight * 4;

	return imageData;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanTexture::CreateImage(const VulkanContext* pContext, const std::string& filename, VkFormat format)
{
	// Load image data!
	stbi_uc* imgData = LoadImageData(pContext, filename);
	
	// Create staging buffer to hold the loaded data, ready to copy to device
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;

	CHECK(pContext->CreateBuffer(	m_vkTextureDeviceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
									&imageStagingBuffer, &imageStagingBufferMemory));

	// Copy image data to staging buffer
	void* data;
	vkMapMemory(pContext->vkDevice, imageStagingBufferMemory, 0, m_vkTextureDeviceSize, 0, &data);
	memcpy(data, imgData, static_cast<uint32_t>(m_vkTextureDeviceSize));
	vkUnmapMemory(pContext->vkDevice, imageStagingBufferMemory);

	// Free original image data
	stbi_image_free(imgData);

	// Create Image...
	CHECK(	pContext->CreateImage2D(m_iTextureWidth, 
									m_iTextureHeight, 
									format, 
									VK_IMAGE_TILING_OPTIMAL, 
									VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
									&(m_Image.image), 
									&(m_Image.deviceMemory)));

	// Transition image to be DST for copy operation
	pContext->TransitionImageLayout(m_Image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// COPY DATA TO IMAGE!
	pContext->CopyImageBuffer(imageStagingBuffer, m_Image.image, m_iTextureWidth, m_iTextureHeight);

	// Transition image to be Shader Readable for shader usage
	pContext->TransitionImageLayout(m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Destroy staging buffers
	vkDestroyBuffer(pContext->vkDevice, imageStagingBuffer, nullptr);
	vkFreeMemory(pContext->vkDevice, imageStagingBufferMemory, nullptr);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanTexture::CreateTextureSampler(const VulkanContext* pContext)
{
	//-- Sampler creation Info
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;								// how to render when image is magnified on screen
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;								// how to render when image is minified on screen			
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;			// how to handle texture wrap in U (x) direction
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;			// how to handle texture wrap in V (y) direction
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;			// how to handle texture wrap in W (z) direction
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;			// border beyond texture (only works for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;						// whether values of texture coords between [0,1] i.e. normalized
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = 0.0f;										// Level of detail bias for mip level
	samplerCreateInfo.minLod = 0.0f;											// minimum level of detail to pick mip level
	samplerCreateInfo.maxLod = 0.0f;											// maximum level of detail to pick mip level
	samplerCreateInfo.anisotropyEnable = VK_FALSE;								// Enable Anisotropy or not? Check physical device features to see if anisotropy is supported or not!
	samplerCreateInfo.maxAnisotropy = 16;										// Anisotropy sample level

	if (vkCreateSampler(pContext->vkDevice, &samplerCreateInfo, nullptr, &m_vkTextureSampler) != VK_SUCCESS)
	{
		LOG_ERROR("Failed to create Texture sampler!");
		return false;
	}

	return true;
}
