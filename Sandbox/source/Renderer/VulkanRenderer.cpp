#include "sandboxPCH.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "Core/Core.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_pRC = new RenderContext();
	m_pVulkanDevice = nullptr;
	m_pFrameBuffer = nullptr;

	m_uiCurrentFrame = 0;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pFrameBuffer);
	SAFE_DELETE(m_pVulkanDevice);
	SAFE_DELETE(m_pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Cleanup()
{
	vkDeviceWaitIdle(m_pRC->vkDevice);
	
	for (uint16_t i = 0; i < Helper::App::gMaxFramesDraws; i++)
	{
		vkDestroySemaphore(m_pRC->vkDevice, m_vkListSemaphoreImageAvailable[i], nullptr);
		vkDestroySemaphore(m_pRC->vkDevice, m_vkListSemaphoreRenderFinished[i], nullptr);
		vkDestroyFence(m_pRC->vkDevice, m_vkListFences[i], nullptr);
	}

	vkDestroyPipeline(m_pRC->vkDevice, m_pRC->vkForwardRenderingPipeline, nullptr);
	vkDestroyPipelineLayout(m_pRC->vkDevice, m_pRC->vkForwardRenderingPipelineLayout, nullptr);

	m_pFrameBuffer->Cleanup(m_pRC);

	vkDestroyRenderPass(m_pRC->vkDevice, m_pRC->vkForwardRenderingRenderPass, nullptr);
	
	vkDestroySurfaceKHR(m_pRC->vkInst, m_pRC->vkSurface, nullptr);
	m_pVulkanDevice->Cleanup(m_pRC);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateVulkanDevice()
{
	m_pVulkanDevice = new VulkanDevice(m_pRC);
	CHECK(m_pVulkanDevice->SetupDevice(m_pRC));

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateFrameBuffers()
{
	m_pFrameBuffer = new VulkanFrameBuffer();
	CHECK(m_pFrameBuffer->CreateFramebuffers(m_pRC));
	
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateCommandBuffers()
{	
	CHECK(m_pVulkanDevice->CreateCommandPool(m_pRC));
	CHECK(m_pVulkanDevice->CreateCommandBuffers(m_pRC));
	
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::Initialize(GLFWwindow* pWindow, VkInstance instance)
{
	m_pRC->vkInst = instance;
	m_pRC->pWindow = pWindow;

	// Create surface
	VK_CHECK(glfwCreateWindowSurface(instance, pWindow, nullptr, &(m_pRC->vkSurface)));

	CHECK(CreateVulkanDevice());
	CHECK(CreateRenderPass());
	CHECK(CreateFrameBuffers());
	CHECK(CreateGraphicsPipeline(Helper::App::FORWARD));
	CHECK(CreateCommandBuffers());
	CHECK(RecordCommands());
	CHECK(CreateSynchronization());
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(float dt)
{

}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Render()
{
	// -- GET NEXT IMAGE
	// 
	vkWaitForFences(m_pRC->vkDevice, 1, &m_vkListFences[m_uiCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_pRC->vkDevice, 1, &m_vkListFences[m_uiCurrentFrame]);

	// Get index of next image to be drawn to & signal semaphore when ready to be drawn to!
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(	m_pRC->vkDevice, m_pRC->vkSwapchain, std::numeric_limits<uint64_t>::max(), 
												m_vkListSemaphoreImageAvailable[m_uiCurrentFrame],
												VK_NULL_HANDLE, &imageIndex);

	// During any event such as window size change etc. we need to check if swap chain recreation is necessary
	// Vulkan tells us that swap chain in no longer adequate during presentation
	// VK_ERROR_OUT_OF_DATE_KHR = swap chain has become incompatible with the surface & can no longer be used for rendering. (window resize)
	// VK_SUBOPTIMAL_KHR = swap chain can be still used to present to the surface but the surface properties are no longer matching!

	// if swap chain is out of date while acquiring the image, then its not possible to present it!
	// We should recreate the swap chain & try again in the next draw call...
	//if (result == VK_ERROR_OUT_OF_DATE_KHR)
	//{
	//	HandleWindowsResize();
	//	return;
	//}
	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	//{
	//	LOG_ERROR("Failed to acquire swapchain image!");
	//	return;
	//}


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
	submitInfo.pCommandBuffers = &(m_pRC->vkListGraphicsCommandBuffers[imageIndex]);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &(m_vkListSemaphoreRenderFinished[m_uiCurrentFrame]);					// semaphores to SIGNAL
	
	// Submit the command buffer to Graphics Queue!
	result = vkQueueSubmit(m_pRC->vkQueueGraphics, 1, &submitInfo, m_vkListFences[m_uiCurrentFrame]);
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
	presentInfo.pSwapchains = &(m_pRC->vkSwapchain);
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_pRC->vkQueuePresent, &presentInfo);
	if (result != VK_SUCCESS)
	{
		LOG_ERROR("Failed to Present Image!");
		return;
	}

	m_uiCurrentFrame = (m_uiCurrentFrame + 1) % Helper::App::gMaxFramesDraws;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::CleanupOnWindowsResize()
{
	vkDestroyPipeline(m_pRC->vkDevice, m_pRC->vkForwardRenderingPipeline, nullptr);
	vkDestroyPipelineLayout(m_pRC->vkDevice, m_pRC->vkForwardRenderingPipelineLayout, nullptr);

	vkDestroyRenderPass(m_pRC->vkDevice, m_pRC->vkForwardRenderingRenderPass, nullptr);

	m_pFrameBuffer->CleanupOnWindowsResize(m_pRC);
	m_pVulkanDevice->CleanupOnWindowsResize(m_pRC);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::HandleWindowsResize()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_pRC->pWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_pRC->pWindow, &width, &height);
		glfwWaitEvents();
	}

	// we shouldn't touch resources that are still in use!
	vkDeviceWaitIdle(m_pRC->vkDevice);

	// Perform cleanup on old version
	CleanupOnWindowsResize();

	// Recreate...! 
	LOG_DEBUG("Starting to Handle windows resize...");

	m_pVulkanDevice->HandleWindowsResize(m_pRC);
	CreateRenderPass();
	CreateGraphicsPipeline(Helper::App::FORWARD);

	m_pFrameBuffer->HandleWindowResize(m_pRC);
	
	m_pVulkanDevice->CreateCommandBuffers(m_pRC);

	LOG_DEBUG("Windows resized handled gracefully!");
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::RecordCommands()
{
	// Information about how to begin each command buffer
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Information about how to begin the render pass
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_pRC->vkForwardRenderingRenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_pRC->vkSwapchainExtent;

	std::array<VkClearValue, 1> clearValues =
	{
		{0.01f, 0.01f, 0.01f, 1.0f}
	};

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	
	for(uint32_t i = 0; i < m_pRC->vkListGraphicsCommandBuffers.size(); i++)
	{
		renderPassBeginInfo.framebuffer = m_pRC->vkListFramebuffers[i];

		// start recording...
		VK_CHECK(vkBeginCommandBuffer(m_pRC->vkListGraphicsCommandBuffers[i], &cmdBufferBeginInfo));

		// Begin RenderPass
		vkCmdBeginRenderPass(m_pRC->vkListGraphicsCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind pipeline to be used in RenderPass
		vkCmdBindPipeline(m_pRC->vkListGraphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pRC->vkForwardRenderingPipeline);

		// Execute
		vkCmdDraw(m_pRC->vkListGraphicsCommandBuffers[i], 3, 1, 0, 0);

		// End RenderPass
		vkCmdEndRenderPass(m_pRC->vkListGraphicsCommandBuffers[i]);

		// end recording...
		VK_CHECK(vkEndCommandBuffer(m_pRC->vkListGraphicsCommandBuffers[i]));

		LOG_INFO("Framebuffer{0} command recorded", i);
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateSynchronization()
{
	m_vkListSemaphoreImageAvailable.resize(Helper::App::gMaxFramesDraws);
	m_vkListSemaphoreRenderFinished.resize(Helper::App::gMaxFramesDraws);
	m_vkListFences.resize(Helper::App::gMaxFramesDraws);

	// Semaphore creation info
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation info
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint16_t i = 0; i < Helper::App::gMaxFramesDraws; i++)
	{
		VK_CHECK(vkCreateSemaphore(m_pRC->vkDevice, &semaphoreCreateInfo, nullptr, &(m_vkListSemaphoreImageAvailable[i])));
		VK_CHECK(vkCreateSemaphore(m_pRC->vkDevice, &semaphoreCreateInfo, nullptr, &(m_vkListSemaphoreRenderFinished[i])));
		VK_CHECK(vkCreateFence(m_pRC->vkDevice, &fenceCreateInfo, nullptr, &(m_vkListFences[i])));
	}
	

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::CreateGraphicsPipeline(Helper::App::ePipeline pipeline)
{
	switch (pipeline)
	{
		case Helper::App::FORWARD:
		{
			// Read shader code & create modules
			VkShaderModule vsModule = Helper::Vulkan::CreateShaderModule(m_pRC->vkDevice, "Assets/Shaders/triangle.vert.spv");
			VkShaderModule fsModule = Helper::Vulkan::CreateShaderModule(m_pRC->vkDevice, "Assets/Shaders/triangle.frag.spv");

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

			// Vertex Input (TODO)
			VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
			vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
			vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
			vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
			vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
			
			// Input Assembly
			VkPipelineInputAssemblyStateCreateInfo inputASCreateInfo = {};
			inputASCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputASCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputASCreateInfo.primitiveRestartEnable = VK_FALSE;

			// Viewport & Scissor
			VkViewport vp = {};
			vp.x = 0.0f;
			vp.y = 0.0f;
			vp.width = static_cast<float>(m_pRC->vkSwapchainExtent.width);
			vp.height = static_cast<float>(m_pRC->vkSwapchainExtent.height);
			vp.maxDepth = 1.0f;
			vp.minDepth = 0.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = m_pRC->vkSwapchainExtent;

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
			rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 0;
			pipelineLayoutCreateInfo.pSetLayouts = nullptr;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

			VK_CHECK(vkCreatePipelineLayout(m_pRC->vkDevice, &pipelineLayoutCreateInfo, nullptr, &(m_pRC->vkForwardRenderingPipelineLayout)));

			// TODO (Setup Depth Stencil testing)

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
			forwardRenderingPipelineInfo.pDepthStencilState = nullptr;
			forwardRenderingPipelineInfo.layout = m_pRC->vkForwardRenderingPipelineLayout;
			forwardRenderingPipelineInfo.renderPass = m_pRC->vkForwardRenderingRenderPass;
			forwardRenderingPipelineInfo.subpass = 0;
			forwardRenderingPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			forwardRenderingPipelineInfo.basePipelineIndex = -1;

			//--  Create Graphics Pipeline!!
			VK_CHECK(vkCreateGraphicsPipelines(m_pRC->vkDevice, VK_NULL_HANDLE, 1, &forwardRenderingPipelineInfo, nullptr, &(m_pRC->vkForwardRenderingPipeline)));

			LOG_DEBUG("Forward Graphics Pipeline created!");

			// Destroy shader module
			vkDestroyShaderModule(m_pRC->vkDevice, fsModule, nullptr);
			vkDestroyShaderModule(m_pRC->vkDevice, vsModule, nullptr);

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
	// Color attachment of render pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_pRC->vkSwapchainImageFormat;
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

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;

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


	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = 1;
	renderpassCreateInfo.pAttachments = &colorAttachment;
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderpassCreateInfo.pDependencies = subpassDependencies.data();

	VK_CHECK(vkCreateRenderPass(m_pRC->vkDevice, &renderpassCreateInfo, nullptr, &m_pRC->vkForwardRenderingRenderPass));

	LOG_DEBUG("Forward RenderPass Created!");

	return true;
}




//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------