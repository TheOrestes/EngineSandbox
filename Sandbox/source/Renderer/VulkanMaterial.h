#pragma once

#include "glm/glm.hpp"

class VulkanTexture;
class VulkanContext;

//---------------------------------------------------------------------------------------------------------------------
enum class TextureType
{
	TEXTURE_ALBEDO,
	TEXTURE_METALNESS,
	TEXTURE_NORMAL,
	TEXTURE_ROUGHNESS,
	TEXTURE_AO,
	TEXTURE_EMISSIVE,
	TEXTURE_HDRI,
	TEXTURE_ERROR
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanMaterial
{
public:
	VulkanMaterial();
	~VulkanMaterial();

	bool					LoadTexture(const VulkanContext* pContext, const std::string& filePath, TextureType type);
	void					Cleanup(const VulkanContext* pContext);
	void					CleanupOnWindowResize(const VulkanContext* pContext);

public:
	// Has Textures?		
	glm::vec3				m_hasTextureAEN;		// Albedo | Emissive | Normal
	glm::vec3				m_hasTextureRMO;		// Roughness | Metallic | Occlusion

	// Textures
	VulkanTexture*			m_pTextureAlbedo;
	VulkanTexture*			m_pTextureEmission;
	VulkanTexture*			m_pTextureNormal;
	VulkanTexture*			m_pTextureMetalness;
	VulkanTexture*			m_pTextureRoughness;
	VulkanTexture*			m_pTextureOcclusion;
	VulkanTexture*			m_pTextureHDRI;
	VulkanTexture*			m_pTextureError;

	// Properties
	glm::vec4				m_colAlbedo;
	glm::vec4				m_colEmission;
	float					m_fRoughess;
	float					m_fMetallic;
	float					m_fOcclusion;

	uint32_t				m_uiNumTextures;
};

