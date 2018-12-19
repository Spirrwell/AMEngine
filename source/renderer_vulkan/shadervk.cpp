#include "shadervk.hpp"
#include "platform.hpp"
#include "vulkan_helpers.hpp"
#include "materialvk.hpp"

#include <fstream>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

static std::vector< std::byte > readFile( const string &fileName )
{
	std::ifstream file( fileName, std::ios::ate | std::ios::binary );

	if ( !file.is_open() )
		return {};

	size_t fileSize = static_cast< size_t >( file.tellg() );
	std::vector< std::byte > buffer( fileSize );

	file.seekg( 0 );
	file.read( ( char* )buffer.data(), fileSize );

	file.close();
		
	return buffer;
}

ShaderVK::ShaderVK( const string &shaderName ) : m_ShaderName( shaderName )
{
	VulkanApp().AddShader( this );
}

ShaderVK::~ShaderVK()
{
	VulkanApp().RemoveShader( this );
}

void ShaderVK::Init()
{
	InitShaderParams();

	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();

	createDescriptorPool();
	//createDescriptorSets( nullptr );
}

void ShaderVK::Shutdown()
{
	cleanupSwapChainElements();

	if ( m_vkDescriptorPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( vulkan().device, m_vkDescriptorPool, nullptr );
		m_vkDescriptorPool = VK_NULL_HANDLE;
	}

	if ( m_vkDescriptorSetLayout != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorSetLayout( vulkan().device, m_vkDescriptorSetLayout, nullptr );
		m_vkDescriptorSetLayout = VK_NULL_HANDLE;
	}
}

void ShaderVK::cleanupSwapChainElements()
{
	// Swap Chain
	if ( m_vkGraphicsPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( vulkan().device, m_vkGraphicsPipeline, nullptr );
		m_vkGraphicsPipeline = VK_NULL_HANDLE;
	}

	if ( m_vkPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( vulkan().device, m_vkPipelineLayout, nullptr );
		m_vkPipelineLayout = VK_NULL_HANDLE;
	}

	if ( m_vkRenderPass != VK_NULL_HANDLE )
	{
		vkDestroyRenderPass( vulkan().device, m_vkRenderPass, nullptr );
		m_vkRenderPass = VK_NULL_HANDLE;
	}
}

void ShaderVK::recreateSwapChainElements()
{
	cleanupSwapChainElements();

	createRenderPass();
	createGraphicsPipeline();
}

void ShaderVK::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = vulkan().swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VulkanApp().findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array< VkAttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if ( vkCreateRenderPass( vulkan().device, &renderPassInfo, nullptr, &m_vkRenderPass ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create render pass." );
}

void ShaderVK::createDescriptorSetLayout()
{
	const auto &bindings = GetDescriptorSetLayoutBindings();

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size() );
	layoutInfo.pBindings = bindings.data();

	if ( vkCreateDescriptorSetLayout( vulkan().device, &layoutInfo, nullptr, &m_vkDescriptorSetLayout ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor set layout!\n" );
}

void ShaderVK::createGraphicsPipeline()
{
	std::vector< std::byte > shaderCode[ SHADER_COUNT ] =
	{
		readFile( string( GAME_DIR ) + string( "shaders/" ) + m_ShaderName + ".vert.spv" ),
		readFile( string( GAME_DIR ) + string( "shaders/" ) + m_ShaderName + ".frag.spv" ),
	};

	for ( const auto &shader : shaderCode )
		if ( shader.empty() ) throw std::runtime_error( "[Vulkan]Failed to read byte code for shader \"" + m_ShaderName + "\"" );

	VkShaderModule shaderModules[ SHADER_COUNT ] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

	for ( size_t i = 0; i < SHADER_COUNT; ++i )
	{
		shaderModules[ i ] = VulkanApp().createShaderModule( shaderCode[ i ] );

		if ( shaderModules[ i ] == VK_NULL_HANDLE )
			throw std::runtime_error( "[Vulkan]Failed to create shader modules for shader \"" + m_ShaderName + "\"" );
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = shaderModules[ VERTEX_SHADER ];
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = shaderModules[ FRAGMENT_SHADER ];
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = GetVertexBindingDescription();
	const auto &attributeDescriptions = GetVertexAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size() );
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >( vulkan().swapChainExtent.width );
	viewport.height = static_cast< float >( vulkan().swapChainExtent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = vulkan().swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; // TODO: Revisit this
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // TODO: Revisit this
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optioanl
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	/*Alpha Blending*/
	/*colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 3 ] = 0.0f; // Optional

	auto pushConstantRanges = GetPushConstants();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = static_cast< uint32_t >( pushConstantRanges.size() );
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	auto destroyShaderModules = [ & ]()
	{
		for ( auto &shaderModule : shaderModules )
			vkDestroyShaderModule( vulkan().device, shaderModule, nullptr );
	};

	if ( vkCreatePipelineLayout( vulkan().device, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout ) != VK_SUCCESS )
	{
		destroyShaderModules();
		throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = SHADER_COUNT;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = m_vkPipelineLayout;
	pipelineInfo.renderPass = m_vkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if ( vkCreateGraphicsPipelines( vulkan().device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkGraphicsPipeline ) != VK_SUCCESS )
	{
		destroyShaderModules();
		throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
	}

	destroyShaderModules();
}

void ShaderVK::createDescriptorPool()
{
	std::array< VkDescriptorPoolSize, 2 > poolSizes = {};
	poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[ 0 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
	poolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[ 1 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	if ( vkCreateDescriptorPool( vulkan().device, &poolInfo, nullptr, &m_vkDescriptorPool ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor pool!" );
}

void ShaderVK::createDescriptorSets( MaterialVK &material )
{
	std::vector< VkDescriptorSetLayout > layouts( vulkan().swapChainImages.size(), m_vkDescriptorSetLayout );

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_vkDescriptorPool;
	allocInfo.descriptorSetCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
	allocInfo.pSetLayouts = layouts.data();

	material.m_vkDescriptorSets.resize( vulkan().swapChainImages.size() );
	if ( vkAllocateDescriptorSets( vulkan().device, &allocInfo, material.m_vkDescriptorSets.data() ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to allocate descriptor sets!" );

	for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
	{
		auto descriptorWrites = GetDescriptorWrites( material, i );
		vkUpdateDescriptorSets( vulkan().device, static_cast< uint32_t >( descriptorWrites.size() ), descriptorWrites.data(), 0, nullptr );
	}
}