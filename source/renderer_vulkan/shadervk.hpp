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

enum ShaderType : size_t
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	SHADER_COUNT
};

struct DefaultUBO
{
	Matrix4f view;
	Matrix4f proj;
};

struct MVPUniform
{
	Matrix4f mvp;
};

struct VulkanPipelineContext
{
	VkPipelineDepthStencilStateCreateInfo DepthStencilState = {};
	std::vector< VkVertexInputBindingDescription > VertexInputBindingDescriptions;
	std::vector< VkVertexInputAttributeDescription > VertexInputAttributeDescriptions;
	VkShaderModule ShaderModules[ SHADER_COUNT ] = { VK_NULL_HANDLE };
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	VkPipeline Pipeline = VK_NULL_HANDLE;
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
public:
	ShaderVK( const string &shaderName );
	virtual ~ShaderVK();

	virtual void Init();
	virtual void Shutdown();

	virtual void InitPipelineInfo();
	virtual void InitVertexInputBindingDescriptions();
	virtual void InitVertexInputAttributeDescriptions();
	
	void cleanupSwapChainElements();
	void recreateSwapChainElements();

	void createDescriptorSetLayout();
	void createShaderModules();
	void createGraphicsPipeline();

	virtual void createUBOs( MaterialVK &material );
	virtual void createDescriptorPool( MaterialVK &material );
	void createDescriptorSets( MaterialVK &material );

	inline const string &GetShaderName() { return m_ShaderName; }

	virtual void InitShaderParams() = 0;
	virtual const std::vector< VkDescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() = 0;
	virtual const std::vector< VkWriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) = 0; // TODO: Make reference
	virtual const std::vector< VkPushConstantRange > GetPushConstants() { return {}; }
	virtual void recordToCommandBuffer( VkCommandBuffer &commandBuffer, const MeshVK &mesh ) {}

	const std::vector< MaterialParameter_t > &GetMaterialParams() { return m_MaterialParams; }
	const VulkanPipelineContext &PipelineCtx() const { return m_Pipeline; }

//protected:
	std::vector< MaterialParameter_t > m_MaterialParams;
	VkDescriptorSetLayout m_vkDescriptorSetLayout = VK_NULL_HANDLE;

	string m_ShaderName;

protected:
	VulkanPipelineContext m_Pipeline;

private:
	uint32_t computeVertexInputSize();
};

#endif // SHADEVK_HPP