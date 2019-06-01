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

const std::vector< vk::DescriptorSetLayoutBinding > &TestShader::GetDescriptorSetLayoutBindings()
{
	static std::vector< vk::DescriptorSetLayoutBinding > bindings;

	if ( bindings.size() > 0 )
		return bindings;

	vk::DescriptorSetLayoutBinding uboLayoutBinding;
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	vk::DescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	bindings.resize( 2 );
	bindings[ 0 ] = uboLayoutBinding;
	bindings[ 1 ] = samplerLayoutBinding;

	return bindings;
}

const std::vector< vk::WriteDescriptorSet > TestShader::GetDescriptorWrites( MaterialVK &material, size_t imageIndex )
{
	static vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = m_UBO.uniformBuffer[ imageIndex ];
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof( m_UBO.UBO );

	static vk::DescriptorImageInfo imageInfo;
	imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	TextureVK *pTexture = material.GetTexture( "diffuse" );

	if ( pTexture )
	{
		imageInfo.imageView = pTexture->ImageView();
		imageInfo.sampler = pTexture->Sampler();
	}

	std::vector< vk::WriteDescriptorSet > descriptorWrites = {};
	descriptorWrites.resize( 2 );

	descriptorWrites[ 0 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 0 ].dstBinding = 0;
	descriptorWrites[ 0 ].dstArrayElement = 0;
	descriptorWrites[ 0 ].descriptorType = vk::DescriptorType::eUniformBuffer;
	descriptorWrites[ 0 ].descriptorCount = 1;
	descriptorWrites[ 0 ].pBufferInfo = &bufferInfo;

	descriptorWrites[ 1 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 1 ].dstBinding = 1;
	descriptorWrites[ 1 ].dstArrayElement = 0;
	descriptorWrites[ 1 ].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrites[ 1 ].descriptorCount = 1;
	descriptorWrites[ 1 ].pImageInfo = &imageInfo;
	descriptorWrites[ 1 ].pTexelBufferView = nullptr; // Optional

	return descriptorWrites;
}

const std::vector< vk::PushConstantRange > TestShader::GetPushConstants()
{
	vk::PushConstantRange pushConstantRange;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( MVPUniform );
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

	std::vector< vk::PushConstantRange > pushRanges = { pushConstantRange };
	return pushRanges;
}

void TestShader::recordToCommandBuffer( vk::CommandBuffer &commandBuffer, const MeshVK &mesh )
{
	//Matrix4f model = mesh.GetOwningModel() ? mesh.GetOwningModel()->GetModelMatrix() : Matrix4f( 1.0f );
	Matrix4f model = glm::translate( Vector3f( 0.0f, -5.0f, 10.0f ) );

	Matrix4f view = g_vkcam.GetViewMatrix();
	//Matrix4f view = glm::translate( Vector3f( 5.0f, 0.0f, 0.0f ) );
	Matrix4f proj = glm::perspective( glm::radians( 70.0f ), g_pEngine->GetAspectRatio(), 0.01f, 10000.0f );

	proj[ 1 ][ 1 ] *= -1.0f;

	static TestShaderPushConstants pConstants;
	pConstants.mvp = proj * view * model;

	commandBuffer.pushConstants( PipelineCtx().PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof( pConstants ), &pConstants );
}