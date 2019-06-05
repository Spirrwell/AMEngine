#include "testshader.hpp"
#include "vertex.hpp"
#include "vulkan_helpers.hpp"
#include "materialvk.hpp"
#include "texturevk.hpp"
#include "meshvk.hpp"
#include "modelvk.hpp"
#include "camera.hpp"
#include "engine/iengine.hpp"

#include <chrono>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern Camera g_vkcam;

static TestShader s_TestShader( "testShader2" );
extern IEngine *g_pEngine;

void TestShader::Init()
{
	ShaderVK::Init();
	m_UBO.Init();
}

void TestShader::Shutdown()
{
	ShaderVK::Shutdown();
	m_UBO.Shutdown();
}

void TestShader::InitShaderParams()
{
	m_MaterialParams.push_back( MaterialParameter_t { "diffuse", MATP_TEXTURE, "textures/shader/error.png" } );
}

const std::vector< VkDescriptorSetLayoutBinding > &TestShader::GetDescriptorSetLayoutBindings()
{
	static std::vector< VkDescriptorSetLayoutBinding > bindings;

	if ( bindings.size() > 0 )
		return bindings;

	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.resize( 2 );
	bindings[ 0 ] = uboLayoutBinding;
	bindings[ 1 ] = samplerLayoutBinding;

	return bindings;
}

const std::vector< VkWriteDescriptorSet > TestShader::GetDescriptorWrites( MaterialVK &material, size_t imageIndex )
{
	static VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = m_UBO.uniformBuffer[ imageIndex ];
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof( m_UBO.UBO );

	static VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	TextureVK *pTexture = material.GetTexture( "diffuse" );

	if ( pTexture )
	{
		imageInfo.imageView = pTexture->ImageView();
		imageInfo.sampler = pTexture->Sampler();
	}

	std::vector< VkWriteDescriptorSet > descriptorWrites = {};
	descriptorWrites.resize( 2 );

	descriptorWrites[ 0 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 0 ].dstBinding = 0;
	descriptorWrites[ 0 ].dstArrayElement = 0;
	descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[ 0 ].descriptorCount = 1;
	descriptorWrites[ 0 ].pBufferInfo = &bufferInfo;

	descriptorWrites[ 1 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 1 ].dstBinding = 1;
	descriptorWrites[ 1 ].dstArrayElement = 0;
	descriptorWrites[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[ 1 ].descriptorCount = 1;
	descriptorWrites[ 1 ].pImageInfo = &imageInfo;
	descriptorWrites[ 1 ].pTexelBufferView = nullptr; // Optional

	return descriptorWrites;
}

const std::vector< VkPushConstantRange > TestShader::GetPushConstants()
{
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( MVPUniform );
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::vector< VkPushConstantRange > pushRanges = { pushConstantRange };
	return pushRanges;
}

void TestShader::recordToCommandBuffer( VkCommandBuffer &commandBuffer, const MeshVK &mesh )
{
	//Matrix4f model = mesh.GetOwningModel() ? mesh.GetOwningModel()->GetModelMatrix() : Matrix4f( 1.0f );
	Matrix4f model = glm::translate( Vector3f( 0.0f, -5.0f, 10.0f ) );

	Matrix4f view = g_vkcam.GetViewMatrix();
	//Matrix4f view = glm::translate( Vector3f( 5.0f, 0.0f, 0.0f ) );
	Matrix4f proj = glm::perspective( glm::radians( 70.0f ), g_pEngine->GetAspectRatio(), 0.01f, 10000.0f );

	proj[ 1 ][ 1 ] *= -1.0f;

	static TestShaderPushConstants pConstants;
	pConstants.mvp = proj * view * model;

	vkCmdPushConstants( commandBuffer, PipelineCtx().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( pConstants ), &pConstants );
}