#include "shadervk.hpp"
#include "platform.hpp"
#include "vulkan_helpers.hpp"
#include "materialvk.hpp"
#include "vertex.hpp"

#include <fstream>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

static std::vector< std::byte > readFile( const std::filesystem::path &fileName )
{
	std::cout << "Loading Shader File: " << fileName << std::endl;
	std::ifstream file( fileName, std::ifstream::binary );

	if ( !file.is_open() )
		return {};

	uintmax_t fileSize = std::filesystem::file_size( fileName );
	std::vector< std::byte > buffer( fileSize );

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
	InitPipelineInfo();
	InitVertexInputBindingDescriptions();
	InitVertexInputAttributeDescriptions();

	InitShaderParams();

	createDescriptorSetLayout();
	createShaderModules();
	createGraphicsPipeline();

	//createDescriptorPool();
	//createDescriptorSets( nullptr );
}

void ShaderVK::Shutdown()
{
	for ( auto &shaderModule : m_Pipeline.ShaderModules )
	{
		if ( shaderModule != VK_NULL_HANDLE )
		{
			vkDestroyShaderModule( vulkan().device, shaderModule, nullptr );
			shaderModule = VK_NULL_HANDLE;
		}
	}

	cleanupSwapChainElements();

	/*if ( m_vkDescriptorPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( vulkan().device, m_vkDescriptorPool, nullptr );
		m_vkDescriptorPool = VK_NULL_HANDLE;
	}*/

	if ( m_vkDescriptorSetLayout != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorSetLayout( vulkan().device, m_vkDescriptorSetLayout, nullptr );
		m_vkDescriptorSetLayout = VK_NULL_HANDLE;
	}
}

void ShaderVK::InitPipelineInfo()
{
	m_Pipeline.DepthStencilState.depthTestEnable = VK_TRUE;
	m_Pipeline.DepthStencilState.depthWriteEnable = VK_TRUE;
	m_Pipeline.DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	m_Pipeline.DepthStencilState.depthBoundsTestEnable = VK_FALSE;
	m_Pipeline.DepthStencilState.minDepthBounds = 0.0f; // Optional
	m_Pipeline.DepthStencilState.maxDepthBounds = 1.0f; // Optional
	m_Pipeline.DepthStencilState.stencilTestEnable = VK_FALSE;
	m_Pipeline.DepthStencilState.front = {}; // Optional
	m_Pipeline.DepthStencilState.back = {}; // Optional
}

void ShaderVK::InitVertexInputBindingDescriptions()
{
	m_Pipeline.VertexInputBindingDescriptions.resize( 1 );

	m_Pipeline.VertexInputBindingDescriptions[ 0 ].binding = 0;
	m_Pipeline.VertexInputBindingDescriptions[ 0 ].stride = computeVertexInputSize();
	m_Pipeline.VertexInputBindingDescriptions[ 0 ].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void ShaderVK::InitVertexInputAttributeDescriptions()
{
	m_Pipeline.VertexInputAttributeDescriptions.resize( 4 );

	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].location = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].location = 1;
	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].location = 2;
	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].format = VK_FORMAT_R32G32_SFLOAT;
	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );

	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].location = 3;
	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].offset = offsetof( Vertex, normal );
}

void ShaderVK::cleanupSwapChainElements()
{
	// Swap Chain
	if ( m_Pipeline.Pipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( vulkan().device, m_Pipeline.Pipeline, nullptr );
		m_Pipeline.Pipeline = VK_NULL_HANDLE;
	}

	if ( m_Pipeline.PipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( vulkan().device, m_Pipeline.PipelineLayout, nullptr );
		m_Pipeline.PipelineLayout = VK_NULL_HANDLE;
	}

	/*if ( m_vkRenderPass != VK_NULL_HANDLE )
	{
		vkDestroyRenderPass( vulkan().device, m_vkRenderPass, nullptr );
		m_vkRenderPass = VK_NULL_HANDLE;
	}*/
}

void ShaderVK::recreateSwapChainElements()
{
	cleanupSwapChainElements();
	createGraphicsPipeline();
}

void ShaderVK::createDescriptorSetLayout()
{
	const auto &bindings = GetDescriptorSetLayoutBindings();

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size() );
	layoutInfo.pBindings = bindings.data();

	if ( vkCreateDescriptorSetLayout( vulkan().device, &layoutInfo, nullptr, &m_vkDescriptorSetLayout ) != VK_SUCCESS  )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor set layout!\n" );
}

void ShaderVK::createShaderModules()
{
	std::vector< std::byte > shaderCode[ SHADER_COUNT ] =
	{
		readFile( PATHS::GAME / "shaders" / ( m_ShaderName + ".vert.spv" ) ),
		readFile( PATHS::GAME / "shaders" / ( m_ShaderName + ".frag.spv" ) ),
	};

	for ( const auto &shader : shaderCode )
		if ( shader.empty() ) throw std::runtime_error( "[Vulkan]Failed to read byte code for shader \"" + m_ShaderName + "\"" );

	for ( size_t i = 0; i < SHADER_COUNT; ++i )
	{
		m_Pipeline.ShaderModules[ i ] = VulkanApp().createShaderModule( shaderCode[ i ] );

		if ( m_Pipeline.ShaderModules[ i ] == VK_NULL_HANDLE )
			throw std::runtime_error( "[Vulkan]Failed to create shader modules for shader \"" + m_ShaderName + "\"" );
	}
}

void ShaderVK::createGraphicsPipeline()
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = m_Pipeline.ShaderModules[ VERTEX_SHADER ];
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = m_Pipeline.ShaderModules[ FRAGMENT_SHADER ];
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast< uint32_t >( m_Pipeline.VertexInputBindingDescriptions.size() );
	vertexInputInfo.pVertexBindingDescriptions = m_Pipeline.VertexInputBindingDescriptions.data();

	vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( m_Pipeline.VertexInputAttributeDescriptions.size() );
	vertexInputInfo.pVertexAttributeDescriptions = m_Pipeline.VertexInputAttributeDescriptions.data();

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
	//rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
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
	/*colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional*/

	/*Alpha Blending*/
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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

	if ( vkCreatePipelineLayout( vulkan().device, &pipelineLayoutInfo, nullptr, &m_Pipeline.PipelineLayout ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );


	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = SHADER_COUNT;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &m_Pipeline.DepthStencilState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = m_Pipeline.PipelineLayout;
	pipelineInfo.renderPass = vulkan().renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if ( vkCreateGraphicsPipelines( vulkan().device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline.Pipeline ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
}

void ShaderVK::createUBOs( MaterialVK &material )
{

}

void ShaderVK::createDescriptorPool( MaterialVK &material )
{
	std::array< VkDescriptorPoolSize, 2 > poolSizes;
	poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[ 0 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
	poolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[ 1 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	if ( vkCreateDescriptorPool( vulkan().device, &poolInfo, nullptr, &material.m_vkDescriptorPool ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor pool!" );
}

void ShaderVK::createDescriptorSets( MaterialVK &material )
{
	std::vector< VkDescriptorSetLayout > layouts( vulkan().swapChainImages.size(), m_vkDescriptorSetLayout );

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = material.m_vkDescriptorPool;
	allocInfo.descriptorSetCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
	allocInfo.pSetLayouts = layouts.data();

	material.m_vkDescriptorSets.resize( vulkan().swapChainImages.size() );
	if ( auto result = vkAllocateDescriptorSets( vulkan().device, &allocInfo, material.m_vkDescriptorSets.data() ); result != VK_SUCCESS )
	{
		stprintf( "Failed to allocate descriptor sets: %d\n", result );
		return;
	}

	for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
	{
		auto descriptorWrites = GetDescriptorWrites( material, i );
		vkUpdateDescriptorSets( vulkan().device, static_cast< uint32_t >( descriptorWrites.size() ), descriptorWrites.data(), 0, nullptr );
	}
}

uint32_t ShaderVK::computeVertexInputSize()
{
	return static_cast< uint32_t >( sizeof( Vertex ) );
}
