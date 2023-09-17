#include "sandboxPCH.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "VulkanContext.h"
#include "Renderables/VulkanMesh.h"
#include "Renderables/VulkanCube.h"
#include "Renderables/VulkanModel.h"
#include "World/Scene.h"
#include "Core/Core.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_pContext = new VulkanContext();
	m_pVulkanDevice = nullptr;
	m_pFrameBuffer = nullptr;

	m_uiCurrentFrame = 0;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pFrameBuffer);
	SAFE_DELETE(m_pVulkanDevice);
	SAFE_DELETE(m_pContext);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Cleanup()
{
	vkDeviceWaitIdle(m_pContext->vkDevice);

	for (uint16_t i = 0; i < Helper::gMaxFramesDraws; i++)
	{
		vkDestroySemaphore(m_pContext->vkDevice, m_vkListSemaphoreImageAvailable[i], nullptr);
		vkDestroySemaphore(m_pContext->vkDevice, m_vkListSemaphoreRenderFinished[i], nullptr);
		vkDestroyFence(m_pContext->vkDevice, m_vkListFences[i], nullptr);
	}

	vkDestroyPipeline(m_pContext->vkDevice, m_pContext->vkForwardRenderingPipeline, nullptr);
	vkDestroyPipelineLayout(m_pContext->vkDevice, m_pContext->vkForwardRenderingPipelineLayout, nullptr);

	m_pFrameBuffer->Cleanup(m_pContext);

	vkDestroyRenderPass(m_pContext->vkDevice, m_pContext->vkForwardRenderingRenderPass, nullptr);
	
	vkDestroySurfaceKHR(m_pContext->vkInst, m_pContext->vkSurface, nullptr);
	m_pVulkanDevice->Cleanup(m_pContext);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateVulkanDevice()
{
	m_pVulkanDevice = new VulkanDevice(m_pContext); 
	CHECK(m_pVulkanDevice->SetupDevice(m_pContext));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateFrameBufferAttachments()
{
	m_pFrameBuffer = new VulkanFrameBuffer();
	CHECK(m_pFrameBuffer->CreateFramebuffersAttachments(m_pContext));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateFrameBuffers()
{
	CHECK(m_pFrameBuffer->CreateFramebuffers(m_pContext));	
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateCommandBuffers()
{	
	CHECK(m_pVulkanDevice->CreateCommandPool(m_pContext));
	CHECK(m_pVulkanDevice->CreateCommandBuffers(m_pContext));
	
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::Initialize(GLFWwindow* pWindow, VkInstance instance)
{
	m_pContext->vkInst = instance;
	m_pContext->pWindow = pWindow;

	// Create surface
	VK_CHECK(glfwCreateWindowSurface(instance, pWindow, nullptr, &(m_pContext->vkSurface)));
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::PreSceneLoad()
{
	CHECK(CreateVulkanDevice());
	CHECK(CreateFrameBufferAttachments());
	CHECK(CreateRenderPass());	
	CHECK(CreateFrameBuffers());
	CHECK(CreateCommandBuffers());
}

//---------------------------------------------------------------------------------------------------------------------
// Need to create scene before creating Graphics Pipeline!
bool VulkanRenderer::PostSceneLoad(Scene* pScene)
{
	CHECK(CreateGraphicsPipeline(pScene, Helper::FORWARD));
	CHECK(CreateSynchronization());
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(Scene* pScene, float dt)
{
	pScene->Update(dt);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Render(Scene* pScene)
{
	// -- GET NEXT IMAGE
	// 
	vkWaitForFences(m_pContext->vkDevice, 1, &m_vkListFences[m_uiCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_pContext->vkDevice, 1, &m_vkListFences[m_uiCurrentFrame]);

	// Get index of next image to be drawn to & signal semaphore when ready to be drawn to!
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(	m_pContext->vkDevice, m_pContext->vkSwapchain, std::numeric_limits<uint64_t>::max(), 
												m_vkListSemaphoreImageAvailable[m_uiCurrentFrame],
												VK_NULL_HANDLE, &imageIndex);

	// During any event such as window size change etc. we need to check if swap chain recreation is necessary
	// Vulkan tells us that swap chain in no longer adequate during presentation
	// VK_ERROR_OUT_OF_DATE_KHR = swap chain has become incompatible with the surface & can no longer be used for rendering. (window resize)
	// VK_SUBOPTIMAL_KHR = swap chain can be still used to present to the surface but the surface properties are no longer matching!

	// if swap chain is out of date while acquiring the image, then its not possible to present it!
	// We should recreate the swap chain & try again in the next draw call...
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		HandleWindowsResize();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		LOG_ERROR("Failed to acquire swapchain image!");
		return;
	}

	RecordCommands(pScene, imageIndex);

	pScene->UpdateUniforms(m_pContext, imageIndex);

	// -- SUBMIT COMMAND BUFFER TO RENDER
	// We ask for image from the swapchain for drawing, but we need to wait till that image is available 
	// also, we need to wait till our pipeline reaches COLOR_ATTACHMENT_OUTPUT stage.
	// Once we are done with the drawing to the image using graphics command buffer, we need to signal
	// saying that we are done with the drawing to the image & that image is ready to PRESENT!
	
	// Queue submission info
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &(m_vkListSemaphoreImageAvailable[m_uiCurrentFrame]);						// sempahores to WAIT on

	std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(m_pContext->vkListGraphicsCommandBuffers[imageIndex]);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &(m_vkListSemaphoreRenderFinished[m_uiCurrentFrame]);					// semaphores to SIGNAL
	
	// Submit the command buffer to Graphics Queue!
	result = vkQueueSubmit(m_pContext->vkQueueGraphics, 1, &submitInfo, m_vkListFences[m_uiCurrentFrame]);
	if (result != VK_SUCCESS)
	{
		LOG_ERROR("Failed to submit Command buffer to Queue!");
		return;
	}

	// 3. PRESENT RENDER IMAGE TO THE SCREEN!
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &(m_vkListSemaphoreRenderFinished[m_uiCurrentFrame]);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(m_pContext->vkSwapchain);
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_pContext->vkQueuePresent, &presentInfo);
	if (result != VK_SUCCESS)
	{
		LOG_ERROR("Failed to Present Image!");
		return;
	}

	m_uiCurrentFrame = (m_uiCurrentFrame + 1) % Helper::gMaxFramesDraws;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CleanupOnWindowsResize()
{
	m_pContext->CleanupOnWindowsResize();

	m_pFrameBuffer->CleanupOnWindowsResize(m_pContext);
	m_pVulkanDevice->CleanupOnWindowsResize(m_pContext);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::HandleWindowsResize()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_pContext->pWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_pContext->pWindow, &width, &height);
		glfwWaitEvents();
	}

	// we shouldn't touch resources that are still in use!
	vkDeviceWaitIdle(m_pContext->vkDevice);

	// Perform cleanup on old version
	CleanupOnWindowsResize();

	// Recreate...! 
	LOG_DEBUG("Starting to Handle windows resize...");

	m_pVulkanDevice->HandleWindowsResize(m_pContext);
	CreateRenderPass();
	
	// TODO - Commenting for now, we need to see if we can utilise VkPipelineCache object for recreation?
	//CreateGraphicsPipeline(Helper::FORWARD);

	m_pFrameBuffer->HandleWindowResize(m_pContext);
	
	m_pVulkanDevice->CreateCommandBuffers(m_pContext);

	LOG_DEBUG("Windows resized handled gracefully!");
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::RecordCommands(Scene* pScene, uint32_t currentImage)
{
	// Information about how to begin each command buffer
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Information about how to begin the render pass
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_pContext->vkForwardRenderingRenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_pContext->vkSwapchainExtent;

	std::array<VkClearValue, 2> clearValues = {};

	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	
	renderPassBeginInfo.framebuffer = m_pContext->vkListFramebuffers[currentImage];
	
	// start recording...
	vkBeginCommandBuffer(m_pContext->vkListGraphicsCommandBuffers[currentImage], &cmdBufferBeginInfo);
	
	// Begin RenderPass
	vkCmdBeginRenderPass(m_pContext->vkListGraphicsCommandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	// Bind pipeline to be used in RenderPass
	vkCmdBindPipeline(m_pContext->vkListGraphicsCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pContext->vkForwardRenderingPipeline);
	
	pScene->Render(m_pContext, currentImage);
	
	// End RenderPass
	vkCmdEndRenderPass(m_pContext->vkListGraphicsCommandBuffers[currentImage]);
	
	// end recording...
	vkEndCommandBuffer(m_pContext->vkListGraphicsCommandBuffers[currentImage]);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateSynchronization()
{
	m_vkListSemaphoreImageAvailable.resize(Helper::gMaxFramesDraws);
	m_vkListSemaphoreRenderFinished.resize(Helper::gMaxFramesDraws);
	m_vkListFences.resize(Helper::gMaxFramesDraws);

	// Semaphore creation info
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation info
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint16_t i = 0; i < Helper::gMaxFramesDraws; i++)
	{
		VK_CHECK(vkCreateSemaphore(m_pContext->vkDevice, &semaphoreCreateInfo, nullptr, &(m_vkListSemaphoreImageAvailable[i])));
		VK_CHECK(vkCreateSemaphore(m_pContext->vkDevice, &semaphoreCreateInfo, nullptr, &(m_vkListSemaphoreRenderFinished[i])));
		VK_CHECK(vkCreateFence(m_pContext->vkDevice, &fenceCreateInfo, nullptr, &(m_vkListFences[i])));
	}
	

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateGraphicsPipeline(Scene* pScene, Helper::ePipeline pipeline)
{
	switch (pipeline)
	{
		case Helper::FORWARD:
		{
			// Read shader code & create modules
			VkShaderModule vsModule = m_pContext->CreateShaderModule("Assets/Shaders/triangle.vert.spv");
			VkShaderModule fsModule = m_pContext->CreateShaderModule("Assets/Shaders/triangle.frag.spv");

			// Vertex Shader stage creation info
			VkPipelineShaderStageCreateInfo vsCreateInfo = {};
			vsCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vsCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vsCreateInfo.module = vsModule;
			vsCreateInfo.pName = "main";

			// Fragment Shader stage creation info
			VkPipelineShaderStageCreateInfo fsCreateInfo = {};
			fsCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fsCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fsCreateInfo.module = fsModule;
			fsCreateInfo.pName = "main";

			std::array<VkPipelineShaderStageCreateInfo, 2> arrShaderStages = { vsCreateInfo, fsCreateInfo };

			// How the data for a single vertex is as a whole!
			VkVertexInputBindingDescription inputBindingDesc = {};
			inputBindingDesc.binding = 0;
			inputBindingDesc.stride = sizeof(Helper::VertexPNTBT);
			inputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			std::array<VkVertexInputAttributeDescription, 5> attrDesc = {};

			// Position
			attrDesc[0].binding = 0;
			attrDesc[0].location = 0;
			attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc[0].offset = offsetof(Helper::VertexPNTBT, Position);

			// Normal
			attrDesc[1].binding = 0;
			attrDesc[1].location = 1;
			attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc[1].offset = offsetof(Helper::VertexPNTBT, Normal);

			// Tangent
			attrDesc[2].binding = 0;
			attrDesc[2].location = 2;
			attrDesc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc[2].offset = offsetof(Helper::VertexPNTBT, Tangent);

			// BiNormal
			attrDesc[3].binding = 0;
			attrDesc[3].location = 3;
			attrDesc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc[3].offset = offsetof(Helper::VertexPNTBT, BiNormal);

			// UV
			attrDesc[4].binding = 0;
			attrDesc[4].location = 4;
			attrDesc[4].format = VK_FORMAT_R32G32_SFLOAT;
			attrDesc[4].offset = offsetof(Helper::VertexPNTBT, UV);

			// Vertex Input (TODO)
			VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
			vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
			vertexInputCreateInfo.pVertexAttributeDescriptions = attrDesc.data();
			vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
			vertexInputCreateInfo.pVertexBindingDescriptions = &inputBindingDesc;
			
			// Input Assembly
			VkPipelineInputAssemblyStateCreateInfo inputASCreateInfo = {};
			inputASCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputASCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputASCreateInfo.primitiveRestartEnable = VK_FALSE;

			// Viewport & Scissor
			VkViewport vp = {};
			vp.x = 0.0f;
			vp.y = 0.0f;
			vp.width = static_cast<float>(m_pContext->vkSwapchainExtent.width);
			vp.height = static_cast<float>(m_pContext->vkSwapchainExtent.height);
			vp.maxDepth = 1.0f;
			vp.minDepth = 0.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = m_pContext->vkSwapchainExtent;

			VkPipelineViewportStateCreateInfo vpCreateInfo = {};
			vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			vpCreateInfo.viewportCount = 1;
			vpCreateInfo.pViewports = &vp;
			vpCreateInfo.scissorCount = 1;
			vpCreateInfo.pScissors = &scissor;

			// Dynamic States
			//std::vector<VkDynamicState> dynamicStates;
			//dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			//dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
			//
			//VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
			//dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			//dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			//dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

			// Rasterizer
			VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
			rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizerCreateInfo.depthClampEnable = VK_FALSE;
			rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizerCreateInfo.lineWidth = 1.0f;
			rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

			// Multisampling
			VkPipelineMultisampleStateCreateInfo msCreateInfo = {};
			msCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			msCreateInfo.sampleShadingEnable = VK_FALSE;
			msCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			// Blending
			VkPipelineColorBlendAttachmentState colorState = {};
			colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorState.blendEnable = VK_TRUE;
			colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorState.colorBlendOp = VK_BLEND_OP_ADD;
			colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorState.alphaBlendOp = VK_BLEND_OP_ADD;
			
			VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
			colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendCreateInfo.logicOpEnable = VK_FALSE;
			colorBlendCreateInfo.attachmentCount = 1;
			colorBlendCreateInfo.pAttachments = &colorState;

			// Pipeline layout (TODO: Descriptor set layouts)
			std::array<VkDescriptorSetLayout, 1> setLayouts = { pScene->GetFirstModel()->m_vkDescriptorSetLayout };
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
			pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

			VK_CHECK(vkCreatePipelineLayout(m_pContext->vkDevice, &pipelineLayoutCreateInfo, nullptr, &(m_pContext->vkForwardRenderingPipelineLayout)));

			// Depth Stencil 
			VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
			depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilCreateInfo.depthTestEnable = VK_TRUE;
			depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
			depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

			VkGraphicsPipelineCreateInfo forwardRenderingPipelineInfo = {};
			forwardRenderingPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			forwardRenderingPipelineInfo.stageCount = static_cast<uint32_t>(arrShaderStages.size());
			forwardRenderingPipelineInfo.pStages = arrShaderStages.data();
			forwardRenderingPipelineInfo.pVertexInputState = &vertexInputCreateInfo;
			forwardRenderingPipelineInfo.pInputAssemblyState = &inputASCreateInfo;
			forwardRenderingPipelineInfo.pViewportState = &vpCreateInfo;
			forwardRenderingPipelineInfo.pDynamicState = nullptr;
			forwardRenderingPipelineInfo.pRasterizationState = &rasterizerCreateInfo;
			forwardRenderingPipelineInfo.pMultisampleState = &msCreateInfo;
			forwardRenderingPipelineInfo.pColorBlendState = &colorBlendCreateInfo;
			forwardRenderingPipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
			forwardRenderingPipelineInfo.layout = m_pContext->vkForwardRenderingPipelineLayout;
			forwardRenderingPipelineInfo.renderPass = m_pContext->vkForwardRenderingRenderPass;
			forwardRenderingPipelineInfo.subpass = 0;
			forwardRenderingPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			forwardRenderingPipelineInfo.basePipelineIndex = -1;

			//--  Create Graphics Pipeline!!
			VK_CHECK(vkCreateGraphicsPipelines(m_pContext->vkDevice, VK_NULL_HANDLE, 1, &forwardRenderingPipelineInfo, nullptr, &(m_pContext->vkForwardRenderingPipeline)));

			LOG_DEBUG("Forward Graphics Pipeline created!");

			// Destroy shader module
			vkDestroyShaderModule(m_pContext->vkDevice, fsModule, nullptr);
			vkDestroyShaderModule(m_pContext->vkDevice, vsModule, nullptr);

			break;
		}

		default:
			break;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateRenderPass()
{
	//-- ATTACHMENTS
	// 1. Color attachment
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_pContext->vkSwapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachRef = {};
	colorAttachRef.attachment = 0;
	colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// 2. Depth attachment
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = m_pContext->vkDepthImageFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachRef = {};
	depthAttachRef.attachment = 1;
	depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//-- SUBPASSES
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	subpass.pDepthStencilAttachment = &depthAttachRef;

	std::array<VkSubpassDependency, 2> subpassDependencies;

	// 1. UNDEFINED --> COLOR_ATTACHMENT
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// 2. COLOR_ATTACHMENT --> PRESENT_SRC_KHR
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	//-- RENDER PASS!
	std::array<VkAttachmentDescription, 2> renderPassAttachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderpassCreateInfo.pAttachments = renderPassAttachments.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderpassCreateInfo.pDependencies = subpassDependencies.data();

	VK_CHECK(vkCreateRenderPass(m_pContext->vkDevice, &renderpassCreateInfo, nullptr, &m_pContext->vkForwardRenderingRenderPass));

	LOG_DEBUG("Forward RenderPass Created!");

	return true;
}

