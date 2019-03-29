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

	const std::vector< vk::DescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	const std::vector< vk::WriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) override;
	const std::vector< vk::PushConstantRange > GetPushConstants() override;
	void recordToCommandBuffer( vk::CommandBuffer &commandBuffer, const MeshVK &mesh ) override;

private:
	UBOWrapperVK< DefaultUBO > m_UBO;
};

#endif // TESTSHADER_HPP