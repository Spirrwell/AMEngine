#ifndef TESTSHADER_HPP
#define TESTSHADER_HPP

#include "shadervk.hpp"

struct TestShaderPushConstants
{
	Matrix4f mvp;
};

class TestShader : public ShaderVK
{
public:
	using ShaderVK::ShaderVK;

	void Init() override;
	void Shutdown() override;

	void InitShaderParams() override;

	const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	const std::vector< VkWriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) override;
	const std::vector< VkPushConstantRange > GetPushConstants() override;
	void recordToCommandBuffer( VkCommandBuffer &commandBuffer, const MeshVK &mesh ) override;

private:
	UBOWrapperVK< DefaultUBO > m_UBO;
};

#endif // TESTSHADER_HPP