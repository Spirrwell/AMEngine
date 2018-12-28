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

static TestShader s_TestShader( "testShader2" );
extern IEngine *g_pEngine;

TestShaderPushConstants g_tspc;

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

VkVertexInputBindingDescription TestShader::GetVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof( Vertex );
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

const std::vector< VkVertexInputAttributeDescription > &TestShader::GetVertexAttributeDescriptions()
{
	static std::vector< VkVertexInputAttributeDescription > attributeDescriptions;

	if ( attributeDescriptions.size() > 0 )
		return attributeDescriptions;

	attributeDescriptions.resize( 4 );

	attributeDescriptions[ 0 ].binding = 0;
	attributeDescriptions[ 0 ].location = 0;
	attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

	attributeDescriptions[ 1 ].binding = 0;
	attributeDescriptions[ 1 ].location = 1;
	attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

	attributeDescriptions[ 2 ].binding = 0;
	attributeDescriptions[ 2 ].location = 2;
	attributeDescriptions[ 2 ].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );

	attributeDescriptions[ 3 ].binding = 0;
	attributeDescriptions[ 3 ].location = 3;
	attributeDescriptions[ 3 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[ 3 ].offset = offsetof( Vertex, normal );

	return attributeDescriptions;
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

	descriptorWrites[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[ 0 ].dstSet = material.m_vkDescriptorSets[ imageIndex ];
	descriptorWrites[ 0 ].dstBinding = 0;
	descriptorWrites[ 0 ].dstArrayElement = 0;
	descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[ 0 ].descriptorCount = 1;
	descriptorWrites[ 0 ].pBufferInfo = &bufferInfo;

	descriptorWrites[ 1 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
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

void TestShader::recordToCommandBuffer( VkCommandBuffer commandBuffer, const MeshVK &mesh )
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	static Camera cam( Vector3f( 0.0f, -5.0f, 0.0f ) );
	cam.Update();
	cam.UpdateView();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration< float, std::chrono::seconds::period >( currentTime - startTime ).count();

	//Matrix4f model = mesh.GetOwningModel() ? mesh.GetOwningModel()->GetModelMatrix() : Matrix4f( 1.0f );
	Matrix4f model = glm::translate( Vector3f( 0.0f, -5.0f, 10.0f ) );

	Matrix4f view = cam.GetViewMatrix();
	//Matrix4f view = glm::translate( Vector3f( 5.0f, 0.0f, 0.0f ) );
	Matrix4f proj = glm::perspective( glm::radians( 70.0f ), g_pEngine->GetAspectRatio(), 0.01f, 1000.0f );

	proj[ 1 ][ 1 ] *= -1.0f;

	struct MatrixPrinter
	{
		MatrixPrinter( const Matrix4f &mat )
		{
			for ( int i = 0; i < 4; ++i )
				stprintf( "%f %f %f %f\n", mat[ i ][ 0 ], mat[ i ][ 1 ], mat[ i ][ 2 ], mat[ i ][ 3 ] );

			stprintf( "\n" );
		}
	};

	static MatrixPrinter v( view );
	static MatrixPrinter m( model );
	static MatrixPrinter p( proj );

	g_tspc.mvp = proj * view * model;

	vkCmdPushConstants( commandBuffer, m_vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( g_tspc ), &g_tspc );
}