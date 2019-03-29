#include "skyshadervk.hpp"
#include "vertex.hpp"
#include "materialvk.hpp"
#include "camera.hpp"
#include "engine/iengine.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern Camera g_vkcam;

static SkyShaderVK s_SkyShader( "skyShader" );
extern IEngine *g_pEngine;

void SkyShaderVK::InitVertexInputAttributeDescriptions()
{
	m_Pipeline.VertexInputAttributeDescriptions.resize( 1 );

	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].location = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].format = vk::Format::eR32G32B32Sfloat;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );
}

void SkyShaderVK::InitShaderParams()
{
	m_MaterialParams.push_back( MaterialParameter_t { "skybox", MATP_SKYTEXTURE } );
}

vk::PipelineDepthStencilStateCreateInfo SkyShaderVK::GetDepthStencilStateInfo()
{
	vk::PipelineDepthStencilStateCreateInfo depthStencil = ShaderVK::GetDepthStencilStateInfo();
	depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;

	return depthStencil;
}

void SkyShaderVK::createDescriptorPool( MaterialVK &material )
{
	std::array< vk::DescriptorPoolSize, 1 > poolSizes;
	poolSizes[ 0 ].type = vk::DescriptorType::eCombinedImageSampler;
	poolSizes[ 0 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	vk::DescriptorPoolCreateInfo poolInfo = {};
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	auto[ result, descriptorPool ] = vulkan().device.createDescriptorPool( poolInfo, nullptr );

	if ( result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor pool!" );

	material.m_vkDescriptorPool = std::move( descriptorPool );
}

const std::vector< vk::DescriptorSetLayoutBinding > &SkyShaderVK::GetDescriptorSetLayoutBindings()
{
	static std::vector< vk::DescriptorSetLayoutBinding > bindings;

	if ( bindings.size() > 0 )
		return bindings;

	vk::DescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	bindings.resize( 1 );
	bindings[ 0 ] = samplerLayoutBinding;

	return bindings;
}

const std::vector< vk::WriteDescriptorSet > SkyShaderVK::GetDescriptorWrites( MaterialVK &material, size_t imageIndex )
{
	static vk::DescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	TextureVK *pTexture = material.GetTexture( "skybox" );

	if ( pTexture )
	{
		imageInfo.imageView = pTexture->ImageView();
		imageInfo.sampler = pTexture->Sampler();
	}

	std::vector< vk::WriteDescriptorSet > descriptorWrites;
	descriptorWrites.resize( 1 );

	descriptorWrites[ 0 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 0 ].dstBinding = 0;
	descriptorWrites[ 0 ].dstArrayElement = 0;
	descriptorWrites[ 0 ].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrites[ 0 ].descriptorCount = 1;
	descriptorWrites[ 0 ].pImageInfo = &imageInfo;
	descriptorWrites[ 0 ].pTexelBufferView = nullptr; // Optional

	return descriptorWrites;
}

const std::vector< vk::PushConstantRange > SkyShaderVK::GetPushConstants()
{
	vk::PushConstantRange pushConstantRange;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( SkyShaderPushConstants );
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

	std::vector< vk::PushConstantRange > pushRanges = { pushConstantRange };
	return pushRanges;
}

void SkyShaderVK::recordToCommandBuffer( vk::CommandBuffer &commandBuffer, const MeshVK &mesh )
{
	Matrix4f view = Matrix4f( Matrix3f( g_vkcam.GetViewMatrix() ) );
	Matrix4f proj = glm::perspective( glm::radians( 70.0f ), g_pEngine->GetAspectRatio(), 0.01f, 1000.0f );

	proj[ 1 ][ 1 ] *= -1.0f;

	static SkyShaderPushConstants pConstants;
	pConstants.view = view;
	pConstants.projection = proj;

	commandBuffer.pushConstants( Pipeline().PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof( pConstants ), &pConstants );
}