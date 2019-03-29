#include "meshvk.hpp"
#include "renderer_vulkan.hpp"
#include "materialvk.hpp"
#include "shadervk.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

MeshVK::MeshVK( const Vertices &vertices, const Indices &indices, MaterialVK *pMaterial, ModelVK *pOwningModel /*= nullptr*/ )
{
	VulkanApp().AddMesh( *this );

	m_Vertices = vertices;
	m_Indices = indices;
	m_pMaterial = pMaterial;
	m_pOwningModel = pOwningModel;

	if ( m_Indices.size() > 0 )
	{
		m_iDrawType = DT_DrawElements;
	}
	else
	{
		m_iDrawType = DT_DrawArrays;
	}

	createVertexBuffer();
	createIndexBuffer();
	createCommandPool();
	allocateSecondaryCommandBuffers();
}

MeshVK::~MeshVK()
{
	VulkanApp().RemoveMesh( this );
	Shutdown();
}

void MeshVK::Shutdown()
{
	if ( m_vkSecondaryCommandBuffers.size() > 0 )
		vulkan().device.freeCommandBuffers( m_vkCommandPool, static_cast< uint32_t >( m_vkSecondaryCommandBuffers.size() ), m_vkSecondaryCommandBuffers.data() );

	m_vkSecondaryCommandBuffers.clear();

	if ( m_vkCommandPool )
	{
		vulkan().device.destroyCommandPool( m_vkCommandPool, nullptr );
		m_vkCommandPool = nullptr;
	}

	if ( m_vkIndexBuffer )
	{
		vulkan().device.destroyBuffer( m_vkIndexBuffer, nullptr );
		m_vkIndexBuffer = nullptr;
	}

	if ( m_vkIndexBufferMemory )
	{
		vulkan().device.freeMemory( m_vkIndexBufferMemory, nullptr );
		m_vkIndexBufferMemory = nullptr;
	}

	if ( m_vkVertexBuffer )
	{
		vulkan().device.destroyBuffer( m_vkVertexBuffer, nullptr );
		m_vkVertexBuffer = nullptr;
	}

	if ( m_vkVertexBufferMemory )
	{
		vulkan().device.freeMemory( m_vkVertexBufferMemory, nullptr );
		m_vkVertexBufferMemory = nullptr;
	}
}

void MeshVK::Draw( vk::CommandBuffer &commandBuffer, const uint32_t &imageIndex )
{
	return;
	ShaderVK *pShader = m_pMaterial->GetShader();

	// We assume that we're already in the render pass (vkCmdBeginRenderPass)
	commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, pShader->Pipeline().Pipeline );
	vk::Buffer vertexBuffers[] = { m_vkVertexBuffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers( 0, 1, vertexBuffers, offsets );
	if ( m_iDrawType == DT_DrawElements )
		commandBuffer.bindIndexBuffer( m_vkIndexBuffer, 0, vk::IndexType::eUint32 );
	pShader->recordToCommandBuffer( commandBuffer, *this );
	commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pShader->Pipeline().PipelineLayout, 0, 1, &m_pMaterial->m_vkDescriptorSets[ imageIndex ], 0, nullptr );
	if ( m_iDrawType == DT_DrawElements )
		commandBuffer.drawIndexed( static_cast< uint32_t >( m_Indices.size() ), 1, 0, 0, 0 );
	else
		commandBuffer.draw( static_cast< uint32_t >( m_Vertices.size() ), 1, 0, 0 );

}

const vk::CommandBuffer &MeshVK::RecordSecondaryCommandBuffers( vk::CommandBufferInheritanceInfo inheritanceInfo, const uint32_t &imageIndex )
{
	ShaderVK *pShader = m_pMaterial->GetShader();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse | vk::CommandBufferUsageFlagBits::eRenderPassContinue;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	m_vkSecondaryCommandBuffers[ imageIndex ].begin( beginInfo );
		m_vkSecondaryCommandBuffers[ imageIndex ].bindPipeline( vk::PipelineBindPoint::eGraphics, pShader->Pipeline().Pipeline );

		vk::Buffer vertexBuffers[] = { m_vkVertexBuffer };
		vk::DeviceSize offsets[] = { 0 };

		m_vkSecondaryCommandBuffers[ imageIndex ].bindVertexBuffers( 0, 1, vertexBuffers, offsets );

		if ( m_iDrawType == DT_DrawElements )
			m_vkSecondaryCommandBuffers[ imageIndex ].bindIndexBuffer( m_vkIndexBuffer, 0, vk::IndexType::eUint32 );

		pShader->recordToCommandBuffer( m_vkSecondaryCommandBuffers[ imageIndex ], *this );
		m_vkSecondaryCommandBuffers[ imageIndex ].bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pShader->Pipeline().PipelineLayout, 0, 1, &m_pMaterial->m_vkDescriptorSets[ imageIndex ], 0, nullptr );

		if ( m_iDrawType == DT_DrawElements )
			m_vkSecondaryCommandBuffers[ imageIndex ].drawIndexed( static_cast< uint32_t >( m_Indices.size() ), 1, 0, 0, 0 );
		else
			m_vkSecondaryCommandBuffers[ imageIndex ].draw( static_cast< uint32_t >( m_Vertices.size() ), 1, 0, 0 );
	m_vkSecondaryCommandBuffers[ imageIndex ].end();

	return m_vkSecondaryCommandBuffers[ imageIndex ];
}

void MeshVK::createVertexBuffer()
{
	if ( m_Vertices.size() == 0 )
		return;

	vk::DeviceSize bufferSize = sizeof( m_Vertices[ 0 ] ) * m_Vertices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	
	VulkanApp().createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;

	vulkan().device.mapMemory( stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &pData );
		std::memcpy( pData, m_Vertices.data(), static_cast< size_t >( bufferSize ) );
	vulkan().device.unmapMemory( stagingBufferMemory );

	VulkanApp().createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, m_vkVertexBuffer, m_vkVertexBufferMemory );
	VulkanApp().copyBuffer( stagingBuffer, m_vkVertexBuffer, bufferSize );

	vulkan().device.destroyBuffer( stagingBuffer, nullptr );
	vulkan().device.freeMemory( stagingBufferMemory, nullptr );
}

void MeshVK::createIndexBuffer()
{
	if ( m_Indices.size() == 0 )
		return;

	vk::DeviceSize bufferSize = sizeof( m_Indices[ 0 ] ) * m_Indices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	VulkanApp().createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;

	vulkan().device.mapMemory( stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &pData );
		std::memcpy( pData, m_Indices.data(), static_cast< size_t >( bufferSize ) );
	vulkan().device.unmapMemory( stagingBufferMemory );

	VulkanApp().createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, m_vkIndexBuffer, m_vkIndexBufferMemory );
	VulkanApp().copyBuffer( stagingBuffer, m_vkIndexBuffer, bufferSize );

	vulkan().device.destroyBuffer( stagingBuffer, nullptr );
	vulkan().device.freeMemory( stagingBufferMemory, nullptr );
}

void MeshVK::createCommandPool()
{
	auto queueFamilyIndices = VulkanApp().findQueueFamilies( vulkan().physicalDevice );

	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	auto[ result, commandPool ] = vulkan().device.createCommandPool( poolInfo, nullptr );

	if ( result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create command pool." );

	m_vkCommandPool = std::move( commandPool );
}

void MeshVK::allocateSecondaryCommandBuffers()
{
	m_vkSecondaryCommandBuffers.resize( vulkan().swapChainFramebuffers.size(), nullptr );

	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = m_vkCommandPool;
	allocInfo.level = vk::CommandBufferLevel::eSecondary;
	allocInfo.commandBufferCount = static_cast< uint32_t >( vulkan().swapChainFramebuffers.size() );

	auto[ result, commandBuffers ] = vulkan().device.allocateCommandBuffers( allocInfo );

	if ( result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create command buffers." );

	m_vkSecondaryCommandBuffers = std::move( commandBuffers );
}