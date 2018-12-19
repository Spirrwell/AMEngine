#ifndef TESTSHADER_HPP
#define TESTSHADER_HPP

#include "shadervk.hpp"


class TestShader : public ShaderVK
{
public:
	using ShaderVK::ShaderVK;

	void Init() override;
	void Shutdown() override;

	void InitShaderParams() override;

	const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	VkVertexInputBindingDescription GetVertexBindingDescription() override;
	const std::vector< VkVertexInputAttributeDescription > &GetVertexAttributeDescriptions() override;
	const std::vector< VkWriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) override;
	const std::vector< VkPushConstantRange > GetPushConstants() override;

private:
	UBOWrapperVK< DefaultUBO > m_UBO;
};

#endif // TESTSHADER_HPP