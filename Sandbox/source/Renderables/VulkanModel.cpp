#include "sandboxPCH.h"
#include "VulkanModel.h"
#include "Renderer/VulkanTexture.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanMaterial.h"
#include "VulkanMesh.h"
#include "World/Camera.h"
#include "Core/Core.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanModel::VulkanModel()
{
	m_pShaderDataBuffer = nullptr;
	m_vkDescriptorPool = VK_NULL_HANDLE;
	m_vkDescriptorSetLayout = VK_NULL_HANDLE;
	
	m_ListDescriptorSets.clear();
	m_ListMeshes.clear();

	m_pMaterial = nullptr;

	m_strModelName.clear();
	m_vecPosition = glm::vec3(0);
	m_vecRotationAxis = glm::vec3(0, 1, 0);
	m_vecScale = glm::vec3(1.0f);
	m_fRotation = 0.0f;
	m_bUpdate = false;
	m_fCurrentAngle = 0.0f;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanModel::~VulkanModel()
{
	SAFE_DELETE(m_pShaderDataBuffer);
	SAFE_DELETE(m_pMaterial);

	m_vkDescriptorPool = VK_NULL_HANDLE;
	m_vkDescriptorSetLayout = VK_NULL_HANDLE;

	m_ListDescriptorSets.clear();
	m_ListMeshes.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::LoadModel(const VulkanContext* pContext, const std::string& filePath)
{
	std::string fileLoc = "Assets/Models/" + filePath;
	LOG_DEBUG("Loading {0} Model...", fileLoc);

	// cut off any directory information already present
	int idx = filePath.find("/");
	m_strModelName = filePath.substr(0, idx);

	// Import Model scene
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileLoc, aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);
	if (!scene)
		LOG_CRITICAL("Failed to Assimp ReadFile {0} model!", fileLoc);

	// Initialize Material & Shader buffers before loading texture data!
	m_pMaterial = new VulkanMaterial();

	m_pShaderDataBuffer = new UniformDataBuffer();
	m_pShaderDataBuffer->CreateUniformDataBuffers(pContext);

	LoadNode(pContext, scene->mRootNode, scene);

	// Get list o::vector<VulkanMesh> f textures based on materials!
	LoadTextures(pContext, scene);

	if (!SetupDescriptors(pContext))
		LOG_CRITICAL("Failed to setup Model {0} Descriptors!!!", fileLoc);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::LoadNode(const VulkanContext* pContext, aiNode* node, const aiScene* scene)
{
	// Go through each mesh at this node & create it, then add it to our mesh list
	for (uint64_t i = 0; i < node->mNumMeshes; i++)
	{
		m_ListMeshes.push_back(LoadMesh(pContext, scene->mMeshes[node->mMeshes[i]], scene));
	}

	// Go through each node attached to this node & load it, then append their meshes to this node's mesh list
	for (uint64_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(pContext, node->mChildren[i], scene);
		//m_vecMeshes.insert(m_vecMeshes.end(), newList.begin(), newList.end());
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::LoadTextures(const VulkanContext* pContext, const aiScene* scene)
{
	// Go through each material and copy its texture file name
	for (uint32_t i = 0; i < scene->mNumMaterials; i++)
	{
		// Get the material
		aiMaterial* material = scene->mMaterials[i];

		// We use Maya's Stingray PBS material for mapping following textures!
		ExtractTextureFromMaterial(pContext, material, aiTextureType_DIFFUSE);
		ExtractTextureFromMaterial(pContext, material, aiTextureType_NORMAL_CAMERA);
		ExtractTextureFromMaterial(pContext, material, aiTextureType_EMISSION_COLOR);
		ExtractTextureFromMaterial(pContext, material, aiTextureType_METALNESS);
		ExtractTextureFromMaterial(pContext, material, aiTextureType_DIFFUSE_ROUGHNESS);
		ExtractTextureFromMaterial(pContext, material, aiTextureType_AMBIENT_OCCLUSION);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::SetDefaultValues(const VulkanContext* pContext, aiTextureType eType)
{
	// if some texture is missing, we still allow to load the default texture to maintain proper descriptor bindings
	// in shader. Else we would need to manage that dynamically. In shader, for now, we are using boolean flag to decide
	// if we read from texture or use the color from Editor! 
	switch (eType)
	{
		case aiTextureType_DIFFUSE:
		{
			m_pShaderDataBuffer->shaderData.hasTextureAEN.r = 0.0f;
			m_pMaterial->LoadTexture(pContext, "Assets/Textures/Default/MissingAlbedo.png", TextureType::TEXTURE_ALBEDO);
			LOG_WARNING("BaseColor texture not found, using default texture!");
			break;
		}

		case aiTextureType_EMISSION_COLOR:
		{
			m_pShaderDataBuffer->shaderData.hasTextureAEN.g = 0.0f;
			m_pMaterial->LoadTexture(pContext, "Assets/Textures/Default/MissingEmissive.png", TextureType::TEXTURE_EMISSIVE);
			LOG_WARNING("Emissive texture not found, using default texture!");
			break;
		}

		case aiTextureType_NORMAL_CAMERA:
		{
			m_pShaderDataBuffer->shaderData.hasTextureAEN.b = 0.0f;
			m_pMaterial->LoadTexture(pContext, "Assets/Textures/Default/MissingNormal.png", TextureType::TEXTURE_NORMAL);
			LOG_WARNING("Normal texture not found, using default texture!");
			break;
		}

		case aiTextureType_DIFFUSE_ROUGHNESS:
		{
			m_pShaderDataBuffer->shaderData.hasTextureRMO.r = 0.0f;
			m_pMaterial->LoadTexture(pContext, "Assets/Textures/Default/MissingRoughness.png", TextureType::TEXTURE_ROUGHNESS);
			LOG_WARNING("Roughness texture not found, using default texture!");
			break;
		}

		case aiTextureType_METALNESS:
		{
			m_pShaderDataBuffer->shaderData.hasTextureRMO.g = 0.0f;
			m_pMaterial->LoadTexture(pContext, "Assets/Textures/Default/MissingMetalness.png", TextureType::TEXTURE_METALNESS);
			LOG_WARNING("Metalness texture not found, using default texture!");
			break;
		}

		case aiTextureType_AMBIENT_OCCLUSION:
		{
			m_pShaderDataBuffer->shaderData.hasTextureRMO.b = 0.0f;
			m_pMaterial->LoadTexture(pContext, "Assets/Textures/Default/MissingAO.png", TextureType::TEXTURE_AO);
			LOG_WARNING("AO texture not found, using default texture!");
			break;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanModel::ExtractTextureFromMaterial(const VulkanContext* pContext, aiMaterial* pMaterial, aiTextureType eType)
{
	unsigned int nCount = pMaterial->GetTextureCount(eType);

	if (pMaterial->GetTextureCount(eType))
	{
		// get the path of the texture file
		aiString path;
		if (pMaterial->GetTexture(eType, 0, &path) == AI_SUCCESS)
		{
			// cut off any directory information already present
			int idx = std::string(path.data).rfind("/");
			std::string fileName = std::string(path.data).substr(idx + 1);

			// Create filename with folder name which is Model name stored earlier...
			fileName = "Assets/Models/" + m_strModelName + "/" + fileName;

			// Inside shader, if texture is available then we sample texture to get color values else use Color values
			// provided. For roughness, metalness & AO property, we simply multiply texture color * editor value! 
			if (!fileName.empty())
			{
				switch (eType)
				{
					case aiTextureType_DIFFUSE:
					{
						m_pShaderDataBuffer->shaderData.hasTextureAEN.r = 1.0f; 
						CHECK(m_pMaterial->LoadTexture(pContext, fileName, TextureType::TEXTURE_ALBEDO));
						break;
					}

					case aiTextureType_EMISSION_COLOR:
					{
						m_pShaderDataBuffer->shaderData.hasTextureAEN.g = 1.0f;
						CHECK(m_pMaterial->LoadTexture(pContext, fileName, TextureType::TEXTURE_EMISSIVE));
						break;
					}

					case aiTextureType_NORMAL_CAMERA:
					{
						m_pShaderDataBuffer->shaderData.hasTextureAEN.b = 1.0f;
						CHECK(m_pMaterial->LoadTexture(pContext, fileName, TextureType::TEXTURE_NORMAL));
						break;
					}

					case aiTextureType_DIFFUSE_ROUGHNESS:
					{
						m_pShaderDataBuffer->shaderData.hasTextureRMO.r = 1.0f;
						CHECK(m_pMaterial->LoadTexture(pContext, fileName, TextureType::TEXTURE_ROUGHNESS));
						break;
					}

					case aiTextureType_METALNESS:
					{
						m_pShaderDataBuffer->shaderData.hasTextureRMO.g = 1.0f;
						CHECK(m_pMaterial->LoadTexture(pContext, fileName, TextureType::TEXTURE_METALNESS));
						break;
					}

					case aiTextureType_AMBIENT_OCCLUSION:
					{
						m_pShaderDataBuffer->shaderData.hasTextureRMO.b = 1.0f;
						CHECK(m_pMaterial->LoadTexture(pContext, fileName, TextureType::TEXTURE_AO));
						break;
					}
				}
			}
			else
			{
				// If due to some reasons, texture slot is assigned but no filename is mentioned, load default!
				SetDefaultValues(pContext, eType);
			}
		}
	}
	else
	{
		// if particular type texture is not available, load default!
		SetDefaultValues(pContext, eType);
	}
}

//---------------------------------------------------------------------------------------------------------------------
VulkanMesh VulkanModel::LoadMesh(const VulkanContext* pContext, aiMesh* mesh, const aiScene* scene)
{
	std::vector<Helper::VertexPNTBT>	vertices;
	std::vector<uint32_t>				indices;
	std::vector<uint32_t>				textureIDs;

	vertices.resize(mesh->mNumVertices);

	// Loop through each vertex...
	for (uint64_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Set position
		vertices[i].Position = { mesh->mVertices[i].x, mesh->mVertices[i].y,  mesh->mVertices[i].z };

		// Set Normals
		vertices[i].Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		if (mesh->mTangents || mesh->mBitangents)
		{
			vertices[i].Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
			vertices[i].BiNormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
		}

		// Set texture coords (if they exists)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].UV = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].UV = { 0.0f, 0.0f };
		}
	}

	// iterate over indices thorough faces for index data...
	for (uint64_t i = 0; i < mesh->mNumFaces; i++)
	{
		// Get a face
		aiFace face = mesh->mFaces[i];

		// go through face's indices & add to the list
		for (uint16_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Create new mesh with details & return it!
	VulkanMesh newMesh(pContext, vertices, indices);
	return newMesh;
}


//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::Render(const VulkanContext* pContext, uint32_t index)
{
	for (int i = 0; i < m_ListMeshes.size(); ++i)
	{

		VkBuffer vertexBuffers[] = { m_ListMeshes[i].m_vkVertexBuffer };											// Buffers to bind
		VkBuffer indexBuffer = m_ListMeshes[i].m_vkIndexBuffer;
		VkDeviceSize offsets[] = { 0 };																				// offsets into buffers being bound
		vkCmdBindVertexBuffers(pContext->vkListGraphicsCommandBuffers[index], 0, 1, vertexBuffers, offsets);		// Command to bind vertex buffer before drawing with them

		// bind mesh index buffer, with zero offset & using uint32_t type
		vkCmdBindIndexBuffer(pContext->vkListGraphicsCommandBuffers[index], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// bind descriptor sets
		vkCmdBindDescriptorSets(pContext->vkListGraphicsCommandBuffers[index],
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								pContext->vkForwardRenderingPipelineLayout,
								0,
								1,
								&(m_ListDescriptorSets[index]),
								0,
								nullptr);

		// Execute pipeline
		vkCmdDrawIndexed(pContext->vkListGraphicsCommandBuffers[index], m_ListMeshes[i].m_uiIndexCount, 1, 0, 0, 0);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::Update(const Camera* pCamera, float dt)
{
	if (m_bUpdate)
	{
		m_fCurrentAngle += dt * 0.05f;
		if (m_fCurrentAngle > 360.0f) { m_fCurrentAngle = 0.0f; }
	}

	m_fRotation = m_fCurrentAngle;

	m_pShaderDataBuffer->shaderData.matWorld = glm::mat4(1);
	m_pShaderDataBuffer->shaderData.matWorld = glm::translate(m_pShaderDataBuffer->shaderData.matWorld, m_vecPosition);
	m_pShaderDataBuffer->shaderData.matWorld = glm::rotate(m_pShaderDataBuffer->shaderData.matWorld, m_fRotation, m_vecRotationAxis);
	m_pShaderDataBuffer->shaderData.matWorld = glm::scale(m_pShaderDataBuffer->shaderData.matWorld, m_vecScale);

	float aspect = (float)gWindowWidht/ (float)gWindowHeight;
	m_pShaderDataBuffer->shaderData.matProjection = pCamera->m_matProjection;// glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
	m_pShaderDataBuffer->shaderData.matProjection[1][1] *= -1.0f;

	m_pShaderDataBuffer->shaderData.matView = pCamera->m_matView;// glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex)
{
	void* data;
	vkMapMemory(pContext->vkDevice, m_pShaderDataBuffer->listDeviceMemory[imageIndex], 0, sizeof(UniformData), 0, &data);
	memcpy(data, &(m_pShaderDataBuffer->shaderData), sizeof(UniformData));
	vkUnmapMemory(pContext->vkDevice, m_pShaderDataBuffer->listDeviceMemory[imageIndex]);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::Cleanup(VulkanContext* pContext)
{
	m_pShaderDataBuffer->Cleanup(pContext);
	m_pMaterial->Cleanup(pContext);

	std::vector<VulkanMesh>::iterator iter = m_ListMeshes.begin();
	for (; iter != m_ListMeshes.end(); iter++)
	{
		(*iter).Cleanup(pContext);
	}

	vkDestroyDescriptorPool(pContext->vkDevice, m_vkDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(pContext->vkDevice, m_vkDescriptorSetLayout, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanModel::CleanupOnWindowsResize(VulkanContext* pContext)
{
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanModel::SetupDescriptors(const VulkanContext* pContext)
{
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

//---------------------------------------------------------------------------------------------------------------------
bool VulkanModel::CreateDescriptorPool(const VulkanContext* pContext)
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

//---------------------------------------------------------------------------------------------------------------------
bool VulkanModel::CreateDescriptorSetLayout(const VulkanContext* pContext)
{
	std::array<VkDescriptorSetLayoutBinding, 7> layoutBindings;

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

	// Metalness texture
	layoutBindings[2].binding = 2;
	layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[2].descriptorCount = 1;
	layoutBindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[2].pImmutableSamplers = nullptr;

	// Normal texture
	layoutBindings[3].binding = 3;
	layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[3].descriptorCount = 1;
	layoutBindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[3].pImmutableSamplers = nullptr;

	// Roughness texture
	layoutBindings[4].binding = 4;
	layoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[4].descriptorCount = 1;
	layoutBindings[4].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[4].pImmutableSamplers = nullptr;

	// Occlusion texture
	layoutBindings[5].binding = 5;
	layoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[5].descriptorCount = 1;
	layoutBindings[5].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[5].pImmutableSamplers = nullptr;

	// Emission texture
	layoutBindings[6].binding = 6;
	layoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[6].descriptorCount = 1;
	layoutBindings[6].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[6].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutCreateInfo.pBindings = layoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(pContext->vkDevice, &layoutCreateInfo, nullptr, &m_vkDescriptorSetLayout));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanModel::CreateDescriptorSets(const VulkanContext* pContext)
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

		//-- Metalness Texture
		VkDescriptorImageInfo metallicImageInfo = {};
		metallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		metallicImageInfo.imageView = m_pMaterial->m_pTextureMetalness->getVkImageView();
		metallicImageInfo.sampler = m_pMaterial->m_pTextureMetalness->getVkSampler();

		VkWriteDescriptorSet metallicWriteSet = {};
		metallicWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		metallicWriteSet.dstSet = m_ListDescriptorSets[i];
		metallicWriteSet.dstBinding = 2;
		metallicWriteSet.dstArrayElement = 0;
		metallicWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		metallicWriteSet.descriptorCount = 1;
		metallicWriteSet.pImageInfo = &metallicImageInfo;

		//-- Normal Texture
		VkDescriptorImageInfo normalImageInfo = {};
		normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalImageInfo.imageView = m_pMaterial->m_pTextureNormal->getVkImageView();
		normalImageInfo.sampler = m_pMaterial->m_pTextureNormal->getVkSampler();

		VkWriteDescriptorSet normalWriteSet = {};
		normalWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		normalWriteSet.dstSet = m_ListDescriptorSets[i];
		normalWriteSet.dstBinding = 3;
		normalWriteSet.dstArrayElement = 0;
		normalWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalWriteSet.descriptorCount = 1;
		normalWriteSet.pImageInfo = &normalImageInfo;

		//-- Roughness Texture
		VkDescriptorImageInfo roughnessImageInfo = {};
		roughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		roughnessImageInfo.imageView = m_pMaterial->m_pTextureRoughness->getVkImageView();
		roughnessImageInfo.sampler = m_pMaterial->m_pTextureRoughness->getVkSampler();

		VkWriteDescriptorSet roughnessWriteSet = {};
		roughnessWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		roughnessWriteSet.dstSet = m_ListDescriptorSets[i];
		roughnessWriteSet.dstBinding = 4;
		roughnessWriteSet.dstArrayElement = 0;
		roughnessWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		roughnessWriteSet.descriptorCount = 1;
		roughnessWriteSet.pImageInfo = &roughnessImageInfo;

		//-- Occlusion Texture
		VkDescriptorImageInfo occlusionImageInfo = {};
		occlusionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		occlusionImageInfo.imageView = m_pMaterial->m_pTextureOcclusion->getVkImageView();
		occlusionImageInfo.sampler = m_pMaterial->m_pTextureOcclusion->getVkSampler();

		VkWriteDescriptorSet occlusionWriteSet = {};
		occlusionWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		occlusionWriteSet.dstSet = m_ListDescriptorSets[i];
		occlusionWriteSet.dstBinding = 5;
		occlusionWriteSet.dstArrayElement = 0;
		occlusionWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		occlusionWriteSet.descriptorCount = 1;
		occlusionWriteSet.pImageInfo = &occlusionImageInfo;

		//-- Emission Texture
		VkDescriptorImageInfo emissionImageInfo = {};
		emissionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		emissionImageInfo.imageView = m_pMaterial->m_pTextureEmission->getVkImageView();
		emissionImageInfo.sampler = m_pMaterial->m_pTextureEmission->getVkSampler();

		VkWriteDescriptorSet emissionWriteSet = {};
		emissionWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		emissionWriteSet.dstSet = m_ListDescriptorSets[i];
		emissionWriteSet.dstBinding = 6;
		emissionWriteSet.dstArrayElement = 0;
		emissionWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		emissionWriteSet.descriptorCount = 1;
		emissionWriteSet.pImageInfo = &emissionImageInfo;

		// List of all Descriptor set writes!
		std::vector<VkWriteDescriptorSet> listWriteSets = { ubWriteSet, albedoWriteSet, metallicWriteSet, normalWriteSet, 
															roughnessWriteSet, occlusionWriteSet, emissionWriteSet };

		// Update the descriptor sets with buffers/binding info
		vkUpdateDescriptorSets(pContext->vkDevice, static_cast<uint32_t>(listWriteSets.size()), listWriteSets.data(), 0, nullptr);
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void UniformDataBuffer::CreateUniformDataBuffers(const VulkanContext* pContext)
{
	VkDeviceSize bufferSize = sizeof(UniformData);

	// One uniform buffer for each swapchain (or command buffer)
	listBuffers.resize(pContext->uiNumSwapchainImages);
	listDeviceMemory.resize(pContext->uiNumSwapchainImages);

	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages; i++)
	{
		pContext->CreateBuffer(	bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								&listBuffers[i], &listDeviceMemory[i]);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void UniformDataBuffer::Cleanup(const VulkanContext* pContext)
{
	for (uint16_t i = 0; i < pContext->uiNumSwapchainImages; i++)
	{
		vkDestroyBuffer(pContext->vkDevice, listBuffers[i], nullptr);
		vkFreeMemory(pContext->vkDevice, listDeviceMemory[i], nullptr);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void UniformDataBuffer::CleanupOnWindowsResize(const VulkanContext* pContext)
{
}
