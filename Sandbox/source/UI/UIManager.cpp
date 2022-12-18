#include "sandboxPCH.h"
#include "UIManager.h"
#include "Core/Core.h"
#include "Renderer/VulkanContext.h"
#include "vulkan/vulkan.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

//---------------------------------------------------------------------------------------------------------------------
UIManager::UIManager()
{

}

//---------------------------------------------------------------------------------------------------------------------
UIManager::~UIManager()
{
}

//---------------------------------------------------------------------------------------------------------------------
bool UIManager::Initialize(const VulkanContext* pContext)
{
	// create descriptor pool for imgui
	// the size of the pool is oversized, but it's copied from the demo!
	VkDescriptorPoolSize poolSizes[] =
	{
		{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(pContext->vkDevice, &poolInfo, nullptr, &imguiPool));

	// Initialize imgui library
	IMGUI_CHECKVERSION();
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontFromFileTTF("Fonts/SFMono-Regular.otf", 13.0f);

	ImGui::CreateContext();
	CHECK(ImGui_ImplGlfw_InitForVulkan(pContext->pWindow, true));

	// this initializes imgui for vulkan
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = pContext->vkInst;
	initInfo.PhysicalDevice = pContext->vkPhysicalDevice;
	initInfo.Device = pContext->vkDevice;
	initInfo.Queue = pContext->vkQueueGraphics;
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = pContext->uiNumSwapchainImages;
	initInfo.ImageCount = pContext->uiNumSwapchainImages;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.CheckVkResultFn = nullptr;

	CHECK(ImGui_ImplVulkan_Init(&initInfo, pContext->vkForwardRenderingRenderPass));

	// execute GPU commands to upload imgui fonts to textures
	VkCommandBuffer cmdBuffer = pContext->BeginCommandBuffer();
	CHECK(ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer));
	pContext->EndAndSubmitCommandBuffer(cmdBuffer);

	// clear fonts from the cpu memory!
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::HandleWindowResize(VulkanContext* pContext)
{
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::BeginRender(const VulkanContext* pContext)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::EndRender(const VulkanContext* pContext, uint32_t imageIndex)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), pContext->vkListGraphicsCommandBuffers[imageIndex]);
}

//---------------------------------------------------------------------------------------------------------------------
void UIManager::Render(const VulkanContext* pContext)
{
	ImGui::ShowDemoWindow();
}
