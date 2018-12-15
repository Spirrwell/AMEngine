#ifndef TESTSHADER_HPP
#define TESTSHADER_HPP

#include "shadervk.hpp"


class TestShader : public ShaderVK
{
	using ShaderVK::ShaderVK;

	const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	VkVertexInputBindingDescription GetVertexBindingDescription() override;
	const std::vector< VkVertexInputAttributeDescription > &GetVertexAttributeDescriptions() override;
};

#endif // TESTSHADER_HPP