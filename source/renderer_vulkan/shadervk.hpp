#ifndef SHADERVK_HPP
#define SHADERVK_HPP

#include "vulkan_interface.hpp"
#include "vulkan/vulkan.hpp"
#include "string.hpp"
#include "mathdefs.hpp"

#include <vector>

struct DefaultUBO
{
	Matrix4f model;
	Matrix4f view;
	Matrix4f proj;
	Matrix4f mvp;
};

class ShaderVK : vkApp::CVulkanInterface
{
	enum ShaderType : size_t
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		SHADER_COUNT
	};

public:
	ShaderVK( const string &shaderName );
	virtual ~ShaderVK();

	void Init();
	void Shutdown();
	
	void cleanupSwapChainElements();
	void recreateSwapChainElements();

	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSets();

	virtual const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() = 0;
	virtual VkVertexInputBindingDescription GetVertexBindingDescription() = 0;
	virtual const std::vector< VkVertexInputAttributeDescription > &GetVertexAttributeDescriptions() = 0;

private:
	VkRenderPass m_vkRenderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_vkDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_vkDescriptorPool = VK_NULL_HANDLE;
	VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_vkGraphicsPipeline = VK_NULL_HANDLE;
	std::vector< VkBuffer > m_vkUniformBuffers;
	std::vector< VkDeviceMemory > m_vkUniformBuffersMemory;
	std::vector< VkDescriptorSet > m_vkDescriptorSets;

	string m_ShaderName;
};

#endif // SHADEVK_HPP