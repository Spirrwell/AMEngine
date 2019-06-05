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

void SkyShaderVK::InitPipelineInfo()
{
	ShaderVK::InitPipelineInfo();
	m_Pipeline.DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
}

void SkyShaderVK::InitVertexInputAttributeDescriptions()
{
	m_Pipeline.VertexInputAttributeDescriptions.resize( 1 );

	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].binding = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].location = 0;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_Pipeline.VertexInputAttributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );
}

void SkyShaderVK::InitShaderParams()
{
	m_MaterialParams.push_back( MaterialParameter_t { "skybox", MATP_SKYTEXTURE } );
}

void SkyShaderVK::createDescriptorPool( MaterialVK &material )
{
	std::array< VkDescriptorPoolSize, 1 > poolSizes;
	poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[ 0 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast< uint32_t >( vulkan().swapChainImages.size() );

	if ( vkCreateDescriptorPool( vulkan().device, &poolInfo, nullptr, &material.m_vkDescriptorPool ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create descriptor pool!" );
}

const std::vector< VkDescriptorSetLayoutBinding > &SkyShaderVK::GetDescriptorSetLayoutBindings()
{
	static std::vector< VkDescriptorSetLayoutBinding > bindings;

	if ( bindings.size() > 0 )
		return bindings;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.resize( 1 );
	bindings[ 0 ] = samplerLayoutBinding;

	return bindings;
}

const std::vector< VkWriteDescriptorSet > SkyShaderVK::GetDescriptorWrites( MaterialVK &material, size_t imageIndex )
{
	static VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	TextureVK *pTexture = material.GetTexture( "skybox" );

	if ( pTexture )
	{
		imageInfo.imageView = pTexture->ImageView();
		imageInfo.sampler = pTexture->Sampler();
	}

	std::vector< VkWriteDescriptorSet > descriptorWrites;
	descriptorWrites.resize( 1 );

	descriptorWrites[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[ 0 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 0 ].dstBinding = 0;
	descriptorWrites[ 0 ].dstArrayElement = 0;
	descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[ 0 ].descriptorCount = 1;
	descriptorWrites[ 0 ].pImageInfo = &imageInfo;
	descriptorWrites[ 0 ].pTexelBufferView = nullptr; // Optional

	return descriptorWrites;
}

const std::vector< VkPushConstantRange > SkyShaderVK::GetPushConstants()
{
	VkPushConstantRange pushConstantRange;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( SkyShaderPushConstants );
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::vector< VkPushConstantRange > pushRanges = { pushConstantRange };
	return pushRanges;
}

void SkyShaderVK::recordToCommandBuffer( VkCommandBuffer &commandBuffer, const MeshVK &mesh )
{
	Matrix4f view = Matrix4f( Matrix3f( g_vkcam.GetViewMatrix() ) );
	Matrix4f proj = glm::perspective( glm::radians( 70.0f ), g_pEngine->GetAspectRatio(), 0.01f, 1000.0f );

	proj[ 1 ][ 1 ] *= -1.0f;

	static SkyShaderPushConstants pConstants;
	pConstants.view = view;
	pConstants.projection = proj;

	vkCmdPushConstants( commandBuffer, PipelineCtx().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( pConstants ), &pConstants );
}