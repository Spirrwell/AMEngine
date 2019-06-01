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
	vk::PipelineDepthStencilStateCreateInfo DepthStencilState;
	std::vector< vk::VertexInputBindingDescription > VertexInputBindingDescriptions;
	std::vector< vk::VertexInputAttributeDescription > VertexInputAttributeDescriptions;
	vk::ShaderModule ShaderModules[ SHADER_COUNT ];
	vk::PipelineLayout PipelineLayout;
	vk::Pipeline Pipeline;
};

template< typename T >
struct UBOWrapperVK : public vkApp::CVulkanInterface
{
	void Init()
	{
		vk::DeviceSize bufferSize = sizeof( T );

		uniformBuffer.resize( vulkan().swapChainImages.size() );
		uniformBufferMemory.resize( vulkan().swapChainImages.size() );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
			VulkanApp().createBuffer( bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffer[ i ], uniformBufferMemory[ i ] );
	}

	void Shutdown()
	{
		for ( size_t i = 0; i < uniformBuffer.size(); ++i )
		{
			vulkan().device.destroyBuffer( uniformBuffer[ i ], nullptr );
			vulkan().device.freeMemory( uniformBufferMemory[ i ], nullptr );
		}
	}

	std::vector< vk::Buffer > uniformBuffer; // Buffer per swap chain image
	std::vector< vk::DeviceMemory > uniformBufferMemory; // Buffer memory per swap chain image
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
	virtual const std::vector< vk::DescriptorSetLayoutBinding > &GetDescriptorSetLayoutBindings() = 0;
	virtual const std::vector< vk::WriteDescriptorSet > GetDescriptorWrites( MaterialVK &material, size_t imageIndex ) = 0; // TODO: Make reference
	virtual const std::vector< vk::PushConstantRange > GetPushConstants() { return {}; }
	virtual void recordToCommandBuffer( vk::CommandBuffer &commandBuffer, const MeshVK &mesh ) {}

	const std::vector< MaterialParameter_t > &GetMaterialParams() { return m_MaterialParams; }
	const VulkanPipelineContext &PipelineCtx() const { return m_Pipeline; }

//protected:
	std::vector< MaterialParameter_t > m_MaterialParams;
	vk::DescriptorSetLayout m_vkDescriptorSetLayout;

	string m_ShaderName;

protected:
	VulkanPipelineContext m_Pipeline;

private:
	uint32_t computeVertexInputSize();
};

#endif // SHADEVK_HPP