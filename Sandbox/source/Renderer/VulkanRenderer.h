#pragma once

#include "vulkan/vulkan.h"
#include "Core/Core.h"
#include "Utility.h"
#include "GLFW/glfw3.h"

#include "IRenderer.h"

class VulkanContext;
class VulkanDevice;
class VulkanFrameBuffer;
class VulkanCube;
class VulkanModel;
class Scene;

//---------------------------------------------------------------------------------------------------------------------
class VulkanRenderer : public IRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool								Initialize(GLFWwindow* pWindow, VkInstance instance);
	bool								PreSceneLoad();
	bool								PostSceneLoad(Scene* pScene);
	void								Update(Scene* pScene, float dt);
	void								Render(Scene* pScene);
	void								HandleWindowsResize();
	void								RecordCommands(Scene* pScene, uint32_t currentImage);
	bool								CreateSynchronization();
	void								Cleanup();
	void								CleanupOnWindowsResize();

	inline	VulkanContext*				GetVulkanContext() const { return m_pContext; }

private:
	bool								CreateVulkanDevice();
	bool								CreateFrameBufferAttachments();
	bool								CreateFrameBuffers();
	bool								CreateCommandBuffers();
	bool								CreateGraphicsPipeline(Scene* pScene, Helper::ePipeline pipeline);
	bool								CreateRenderPass();

private:
	VulkanContext*						m_pContext;
	VulkanDevice*						m_pVulkanDevice;
	VulkanFrameBuffer*					m_pFrameBuffer;

	// -- Synchronization!
	uint32_t							m_uiCurrentFrame;
	std::vector<VkSemaphore>			m_vkListSemaphoreImageAvailable;
	std::vector<VkSemaphore>			m_vkListSemaphoreRenderFinished;
	std::vector<VkFence>				m_vkListFences;
};

