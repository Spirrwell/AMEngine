#ifndef TESTSHADER_HPP
#define TESTSHADER_HPP

#include "shadervk.hpp"


class TestShader : public ShaderVK
{
public:
	using ShaderVK::ShaderVK;

	void InitShaderParams() override;

	const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	VkVertexInputBindingDescription GetVertexBindingDescription() override;
	const std::vector< VkVertexInputAttributeDescription > &GetVertexAttributeDescriptions() override;
	const std::vector< VkWriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) override;
};

#endif // TESTSHADER_HPP