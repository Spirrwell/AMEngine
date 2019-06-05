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
	const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() override;
	const std::vector< VkWriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) override;
	const std::vector< VkPushConstantRange > GetPushConstants() override;
	void recordToCommandBuffer( VkCommandBuffer &commandBuffer, const MeshVK &mesh ) override;
};

#endif // SKYSHADERVK_HPP