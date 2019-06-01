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
	std::basic_ifstream< std::byte > file( fileName, std::ios::binary );

	if ( !file.is_open() )
		return {};

	uintmax_t fileSize = std::filesystem::file_size( fileName );
	std::vector< std::byte > buffer( fileSize );

	file.read( buffer.data(), fileSize );

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
		if ( shaderModule )
		{
			vulkan().device.destroyShaderModule( shaderModule, nullptr );
			shaderModule = nullptr;
		}
	}

	cleanupSwapChainElements();

	/*if ( m_vkDescriptorPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( vulkan().device, m_vkDescriptorPool, nullptr );
		m_vkDescriptorPool = VK_NULL_HANDLE;
	}*/

	if ( m_vkDescriptorSetLayout )
	{
		vulkan().device.destroyDescriptorSetLayout( m_vkDescriptorSetLayout, nullptr );
		m_vkDescriptorSetLayout = nullptr;
	}
}

void ShaderVK::InitPipelineInfo()
{
	m_Pipeline.DepthStencilState.depthTestEnable = VK_TRUE;
	m_Pipeline.DepthStencilState.depthWriteEnable = VK_TRUE;
	m_Pipeline.DepthStencilState.depthCompareOp = vk::CompareOp::eLess;
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
	m_Pipeline.VertexInputBindingDescriptions[ 0 ].inputRate = vk::VertexInputRate::eVertex;
}

void ShaderVK::InitVertexInputAttributeDescriptions()
{
	m_Pipeline.VertexInputAttributeDescriptions.resize( 4 );

	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].location = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].format = vk::Format::eR32G32B32Sfloat;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].location = 1;
	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].format = vk::Format::eR32G32B32Sfloat;
	m_Pipeline.VertexInputAttributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].location = 2;
	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].format = vk::Format::eR32G32Sfloat;
	m_Pipeline.VertexInputAttributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );

	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].location = 3;
	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].format = vk::Format::eR32G32B32Sfloat;
	m_Pipeline.VertexInputAttributeDescriptions[ 3 ].offset = offsetof( Vertex, normal );
}

void ShaderVK::cleanupSwapChainElements()
{
	// Swap Chain
	if ( m_Pipeline.Pipeline )
	{
		vulkan().device.destroyPipeline( m_Pipeline.Pipeline, nullptr );
		m_Pipeline.Pipeline = nullptr;
	}

	if ( m_Pipeline.PipelineLayout )
	{
		vulkan().device.destroyPipelineLayout( m_Pipeline.PipelineLayout, nullptr );
		m_Pipeline.PipelineLayout = nullptr;
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

	vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size() );
	layoutInfo.pBindings = bindings.data();

	if ( auto[ result, descriptorSetLayout ] = vulkan().device.createDescriptorSetLayout( layoutInfo, nullptr ); result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor set layout!\n" );
	else
		m_vkDescriptorSetLayout = std::move( descriptorSetLayout );
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

		if ( !m_Pipeline.ShaderModules[ i ] )
			throw std::runtime_error( "[Vulkan]Failed to create shader modules for shader \"" + m_ShaderName + "\"" );
	}
}

void ShaderVK::createGraphicsPipeline()
{
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = m_Pipeline.ShaderModules[ VERTEX_SHADER ];
	vertShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = m_Pipeline.ShaderModules[ FRAGMENT_SHADER ];
	fragShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

	vertexInputInfo.vertexBindingDescriptionCount = static_cast< uint32_t >( m_Pipeline.VertexInputBindingDescriptions.size() );
	vertexInputInfo.pVertexBindingDescriptions = m_Pipeline.VertexInputBindingDescriptions.data();

	vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( m_Pipeline.VertexInputAttributeDescriptions.size() );
	vertexInputInfo.pVertexAttributeDescriptions = m_Pipeline.VertexInputAttributeDescriptions.data();

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >( vulkan().swapChainExtent.width );
	viewport.height = static_cast< float >( vulkan().swapChainExtent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = vulkan().swapChainExtent;

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	//rasterizer.polygonMode = vk::PolygonMode::eLine;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eFront; // TODO: Revisit this
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise; // TODO: Revisit this
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optioanl
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	/*colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional*/

	/*Alpha Blending*/
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 3 ] = 0.0f; // Optional

	auto pushConstantRanges = GetPushConstants();

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = static_cast< uint32_t >( pushConstantRanges.size() );
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	if ( auto[ result, pipelineLayout ] = vulkan().device.createPipelineLayout( pipelineLayoutInfo, nullptr ); result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
	else
		m_Pipeline.PipelineLayout = std::move( pipelineLayout );


	vk::GraphicsPipelineCreateInfo pipelineInfo;
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

	if ( auto[ result, graphicsPipeline ] = vulkan().device.createGraphicsPipeline( nullptr, pipelineInfo, nullptr ); result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
	else
		m_Pipeline.Pipeline = std::move( graphicsPipeline );
}

void ShaderVK::createUBOs( MaterialVK &material )
{

}

void ShaderVK::createDescriptorPool( MaterialVK &material )
{
	std::array< vk::DescriptorPoolSize, 2 > poolSizes;
	poolSizes[ 0 ].type = vk::DescriptorType::eUniformBuffer;
	poolSizes[ 0 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
	poolSizes[ 1 ].type = vk::DescriptorType::eCombinedImageSampler;
	poolSizes[ 1 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	if ( auto[ result, descriptorPool ] = vulkan().device.createDescriptorPool( poolInfo, nullptr ); result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor pool!" );
	else
		material.m_vkDescriptorPool = std::move( descriptorPool );
}

void ShaderVK::createDescriptorSets( MaterialVK &material )
{
	std::vector< vk::DescriptorSetLayout > layouts( vulkan().swapChainImages.size(), m_vkDescriptorSetLayout );

	vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.descriptorPool = material.m_vkDescriptorPool;
	allocInfo.descriptorSetCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
	allocInfo.pSetLayouts = layouts.data();

	material.m_vkDescriptorSets.resize( vulkan().swapChainImages.size() );
	if ( auto[ result, descriptorSets ] = vulkan().device.allocateDescriptorSets( allocInfo ); result != vk::Result::eSuccess )
	{
		stprintf( "Failed to allocate descriptor sets: %d\n", result );
		return;
	}
	else
		material.m_vkDescriptorSets = std::move( descriptorSets );

	for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
	{
		auto descriptorWrites = GetDescriptorWrites( material, i );
		vulkan().device.updateDescriptorSets( static_cast< uint32_t >( descriptorWrites.size() ), descriptorWrites.data(), 0, nullptr );
	}
}

uint32_t ShaderVK::computeVertexInputSize()
{
	return static_cast< uint32_t >( sizeof( Vertex ) );
}