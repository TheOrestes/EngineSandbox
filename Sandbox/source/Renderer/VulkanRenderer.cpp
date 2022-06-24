#include "sandboxPCH.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	m_pRC = new RenderContext();
	m_pVulkanDevice = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	SAFE_DELETE(m_pRC);
	SAFE_DELETE(m_pVulkanDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Destroy()
{
	vkDestroySurfaceKHR(m_pRC->vkInst, m_pRC->vkSurface, nullptr);
	m_pVulkanDevice->Destroy();
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanRenderer::Initialize(GLFWwindow* pWindow, VkInstance instance)
{
	m_pRC->vkInst = instance;
	m_pRC->pWindow = pWindow;

	// Create surface
	VK_CHECK(glfwCreateWindowSurface(instance, pWindow, nullptr, &(m_pRC->vkSurface)));

	// Create Vulkan device...
	m_pVulkanDevice = new VulkanDevice(m_pRC);
	CHECK(m_pVulkanDevice->SetupDevice(m_pRC));
	CHECK(CreateRenderPass());
	CHECK(CreateGraphicsPipeline(Helper::App::FORWARD));
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Update(float dt)
{

}

//---------------------------------------------------------------------------------------------------------------------
void VulkanRenderer::Render()
{

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
			//dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT1112568+7+);
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