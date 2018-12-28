#ifndef SHADERVK_HPP
#define SHADERVK_HPP

#include "vulkan_interface.hpp"
#include "vulkan_helpers.hpp"
#include "vulkan/vulkan.hpp"
#include "string.hpp"
#include "mathdefs.hpp"
#include "shadersystem/ishader.hpp"

#include <vector>

class MaterialVK;
class MeshVK;

struct DefaultUBO
{
	Matrix4f view;
	Matrix4f proj;
};

struct MVPUniform
{
	Matrix4f mvp;
};

template< typename T >
struct UBOWrapperVK : public vkApp::CVulkanInterface
{
	void Init()
	{
		VkDeviceSize bufferSize = sizeof( T );

		uniformBuffer.resize( vulkan().swapChainImages.size() );
		uniformBufferMemory.resize( vulkan().swapChainImages.size() );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
			VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer[ i ], uniformBufferMemory[ i ] );
	}

	void Shutdown()
	{
		for ( size_t i = 0; i < uniformBuffer.size(); ++i )
		{
			vkDestroyBuffer( vulkan().device, uniformBuffer[ i ], nullptr );
			vkFreeMemory( vulkan().device, uniformBufferMemory[ i ], nullptr );
		}
	}

	std::vector< VkBuffer > uniformBuffer; // Buffer per swap chain image
	std::vector< VkDeviceMemory > uniformBufferMemory; // Buffer memory per swap chain image
	T UBO;
};

class ShaderVK : public vkApp::CVulkanInterface
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

	virtual void Init();
	virtual void Shutdown();
	
	void cleanupSwapChainElements();
	void recreateSwapChainElements();

	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();

	void createDescriptorPool( MaterialVK &material );
	void createDescriptorSets( MaterialVK &material );

	inline const string &GetShaderName() { return m_ShaderName; }

	virtual void InitShaderParams() = 0;
	virtual const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() = 0;
	virtual VkVertexInputBindingDescription GetVertexBindingDescription() = 0;
	virtual const std::vector< VkVertexInputAttributeDescription > &GetVertexAttributeDescriptions() = 0;
	virtual const std::vector< VkWriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) = 0; // TODO: Make reference
	virtual const std::vector< VkPushConstantRange > GetPushConstants() { return {}; }
	virtual void recordToCommandBuffer( VkCommandBuffer commandBuffer, const MeshVK &mesh ) {}

	const std::vector< MaterialParameter_t > &GetMaterialParams() { return m_MaterialParams; }

//protected:
	std::vector< MaterialParameter_t > m_MaterialParams;
	//VkRenderPass m_vkRenderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_vkDescriptorSetLayout = VK_NULL_HANDLE;
	//VkDescriptorPool m_vkDescriptorPool = VK_NULL_HANDLE;
	VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_vkGraphicsPipeline = VK_NULL_HANDLE;
	//std::vector< VkDescriptorSet > m_vkDescriptorSets;

	string m_ShaderName;
};

#endif // SHADEVK_HPP