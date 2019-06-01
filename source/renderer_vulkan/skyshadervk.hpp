#ifndef SKYSHADERVK_HPP
#define SKYSHADERVK_HPP

#include "shadervk.hpp"

struct SkyShaderPushConstants
{
	Matrix4f view;
	Matrix4f projection;
};

class SkyShaderVK : public ShaderVK
{
public:
	using ShaderVK::ShaderVK;

	void InitPipelineInfo() override;
	void InitVertexInputAttributeDescriptions() override;
	void InitShaderParams() override;

	void createDescriptorPool( MaterialVK &material ) override;
	const std::vector< vk::DescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	const std::vector< vk::WriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) override;
	const std::vector< vk::PushConstantRange > GetPushConstants() override;
	void recordToCommandBuffer( vk::CommandBuffer &commandBuffer, const MeshVK &mesh ) override;
};

#endif // SKYSHADERVK_HPP