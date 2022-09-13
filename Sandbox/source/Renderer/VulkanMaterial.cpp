#include "sandboxPCH.h"
#include "VulkanTexture.h"
#include "VulkanMaterial.h"
#include "VulkanContext.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanMaterial::VulkanMaterial()
{
	m_pTextureAlbedo = nullptr;
	m_pTextureEmission = nullptr;
	m_pTextureMetalness = nullptr;
	m_pTextureNormal = nullptr;
	m_pTextureOcclusion = nullptr;
	m_pTextureRoughness = nullptr;
	m_pTextureHDRI = nullptr;
	m_pTextureError = nullptr;

	m_hasTextureAEN = glm::vec3(0);
	m_hasTextureRMO = glm::vec3(0);

	m_uiNumTextures = 0;
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanMaterial::~VulkanMaterial()
{
	SAFE_DELETE(m_pTextureAlbedo);
	SAFE_DELETE(m_pTextureEmission);
	SAFE_DELETE(m_pTextureMetalness);
	SAFE_DELETE(m_pTextureNormal);
	SAFE_DELETE(m_pTextureOcclusion);
	SAFE_DELETE(m_pTextureRoughness);
	SAFE_DELETE(m_pTextureHDRI);
	SAFE_DELETE(m_pTextureError);
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMaterial::LoadTexture(const VulkanContext* pContext, const std::string& filePath, TextureType type)
{
	switch (type)
	{
		case TextureType::TEXTURE_ALBEDO:
		{
			m_pTextureAlbedo = new VulkanTexture();
			CHECK(m_pTextureAlbedo->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_SRGB));
			++m_uiNumTextures;
			break;
		}

		case TextureType::TEXTURE_EMISSIVE:
		{
			m_pTextureEmission = new VulkanTexture();
			CHECK(m_pTextureEmission->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_UNORM));
			++m_uiNumTextures;
			break;
		}
			
		case TextureType::TEXTURE_METALNESS:
		{
			m_pTextureMetalness = new VulkanTexture();
			CHECK(m_pTextureMetalness->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_UNORM));
			++m_uiNumTextures;
			break;
		}
			
		case TextureType::TEXTURE_NORMAL:
		{
			m_pTextureNormal = new VulkanTexture();
			CHECK(m_pTextureNormal->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_UNORM));
			++m_uiNumTextures;
			break;
		}
			
		case TextureType::TEXTURE_ROUGHNESS:
		{
			m_pTextureRoughness = new VulkanTexture();
			CHECK(m_pTextureRoughness->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_UNORM));
			++m_uiNumTextures;
			break;
		}
			
		case TextureType::TEXTURE_AO:
		{
			m_pTextureOcclusion = new VulkanTexture();
			CHECK(m_pTextureOcclusion->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_UNORM));
			++m_uiNumTextures;
			break;
		}
			
		case TextureType::TEXTURE_HDRI:
		{
			m_pTextureHDRI = new VulkanTexture();
			CHECK(m_pTextureHDRI->CreateTexture(pContext, filePath, VK_FORMAT_R32G32B32A32_SFLOAT));
			++m_uiNumTextures;
			break;
		}
			
		case TextureType::TEXTURE_ERROR:
		{
			m_pTextureError = new VulkanTexture();
			CHECK(m_pTextureError->CreateTexture(pContext, filePath, VK_FORMAT_R8G8B8A8_UNORM));
			++m_uiNumTextures;
			break;
		}
			
		default:
			break;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMaterial::Cleanup(const VulkanContext* pContext)
{
	if (m_pTextureAlbedo)		{ m_pTextureAlbedo->Cleanup(pContext); }
	if (m_pTextureEmission)		{ m_pTextureEmission->Cleanup(pContext); }
	if (m_pTextureError)		{ m_pTextureError->Cleanup(pContext); }
	if (m_pTextureHDRI)			{ m_pTextureHDRI->Cleanup(pContext); }
	if (m_pTextureMetalness)	{ m_pTextureMetalness->Cleanup(pContext); }
	if (m_pTextureNormal)		{ m_pTextureNormal->Cleanup(pContext); }
	if (m_pTextureOcclusion)	{ m_pTextureOcclusion->Cleanup(pContext); }
	if (m_pTextureRoughness)	{ m_pTextureRoughness->Cleanup(pContext); }
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMaterial::CleanupOnWindowResize(const VulkanContext* pContext)
{
}
