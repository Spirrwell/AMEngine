#include "testshader.hpp"
#include "vertex.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

static TestShader s_TestShader( "testShader" );

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