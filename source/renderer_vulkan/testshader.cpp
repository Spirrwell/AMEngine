#include "testshader.hpp"
#include "vertex.hpp"
#include "vulkan_helpers.hpp"
#include "materialvk.hpp"
#include "texturevk.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

static TestShader s_TestShader( "testShader" );

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
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = m_vkUniformBuffers[ imageIndex ];
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof( DefaultUBO );

	VkDescriptorImageInfo imageInfo = {};
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