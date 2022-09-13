#pragma once

#include "Renderer/Utility.h"

class VulkanContext;
enum class TextureType;

//---------------------------------------------------------------------------------------------------------------------
class VulkanTexture
{
public:
	VulkanTexture();
	~VulkanTexture();

	bool						CreateTexture(const VulkanContext* pContext, const std::string& filename, VkFormat format);
	void						Cleanup(const VulkanContext* pContext);
	void						CleanupOnWindowResize(const VulkanContext* pContext);

public:
	inline VkImage				getVkImage()		{ return m_Image.image; }
	inline VkImageView			getVkImageView()	{ return m_Image.imageView; }
	inline VkSampler			getVkSampler()		{ return m_vkTextureSampler; }

private:
	Helper::VulkanImage			m_Image;
	VkSampler					m_vkTextureSampler;
								
private:						
	unsigned char*				LoadImageData(const VulkanContext* pContext, const std::string& filename);
	bool						CreateImage(const VulkanContext* pContext, const std::string& filename, VkFormat format);
	bool						CreateTextureSampler(const VulkanContext* pContext);
								
	int							m_iTextureWidth;
	int							m_iTextureHeight;
	int							m_iTextureChannels;
	VkDeviceSize				m_vkTextureDeviceSize;
};

