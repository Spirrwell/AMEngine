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
		vkFreeCommandBuffers( vulkan().device, m_vkCommandPool, static_cast< uint32_t >( m_vkSecondaryCommandBuffers.size() ), m_vkSecondaryCommandBuffers.data() );

	m_vkSecondaryCommandBuffers.clear();

	if ( m_vkCommandPool != VK_NULL_HANDLE )
	{
		vkDestroyCommandPool( vulkan().device, m_vkCommandPool, nullptr );
		m_vkCommandPool = VK_NULL_HANDLE;
	}

	if ( m_vkIndexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( vulkan().device, m_vkIndexBuffer, nullptr );
		m_vkIndexBuffer = VK_NULL_HANDLE;
	}

	if ( m_vkIndexBufferMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( vulkan().device, m_vkIndexBufferMemory, nullptr );
		m_vkIndexBufferMemory = VK_NULL_HANDLE;
	}

	if ( m_vkVertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( vulkan().device, m_vkVertexBuffer, nullptr );
		m_vkVertexBuffer = VK_NULL_HANDLE;
	}

	if ( m_vkVertexBufferMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( vulkan().device, m_vkVertexBufferMemory, nullptr );
		m_vkVertexBufferMemory = VK_NULL_HANDLE;
	}
}

void MeshVK::Draw( VkCommandBuffer commandBuffer, const uint32_t &imageIndex )
{
	return;
	ShaderVK *pShader = m_pMaterial->GetShader();

	// We assume that we're already in the render pass (vkCmdBeginRenderPass)
	vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->Pipeline().Pipeline );
	VkBuffer vertexBuffers[] = { m_vkVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( commandBuffer, 0, 1, vertexBuffers, offsets );
	if ( m_iDrawType == DT_DrawElements )
		vkCmdBindIndexBuffer( commandBuffer, m_vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );
	pShader->recordToCommandBuffer( commandBuffer, *this );
	vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->Pipeline().PipelineLayout, 0, 1, &m_pMaterial->m_vkDescriptorSets[ imageIndex ], 0, nullptr );
	if ( m_iDrawType == DT_DrawElements )
		vkCmdDrawIndexed( commandBuffer, static_cast< uint32_t >( m_Indices.size() ), 1, 0, 0, 0 );
	else
		vkCmdDraw( commandBuffer, static_cast< uint32_t >( m_Vertices.size() ), 1, 0, 0 );

}

const VkCommandBuffer &MeshVK::RecordSecondaryCommandBuffers( VkCommandBufferInheritanceInfo inheritanceInfo, const uint32_t &imageIndex )
{
	ShaderVK *pShader = m_pMaterial->GetShader();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer( m_vkSecondaryCommandBuffers[ imageIndex ], &beginInfo );
		vkCmdBindPipeline( m_vkSecondaryCommandBuffers[ imageIndex ], VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->Pipeline().Pipeline );
		VkBuffer vertexBuffers[] = { m_vkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers( m_vkSecondaryCommandBuffers[ imageIndex ], 0, 1, vertexBuffers, offsets );
		if ( m_iDrawType == DT_DrawElements )
			vkCmdBindIndexBuffer( m_vkSecondaryCommandBuffers[ imageIndex ], m_vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );
		pShader->recordToCommandBuffer( m_vkSecondaryCommandBuffers[ imageIndex ], *this );
		vkCmdBindDescriptorSets( m_vkSecondaryCommandBuffers[ imageIndex ], VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->Pipeline().PipelineLayout, 0, 1, &m_pMaterial->m_vkDescriptorSets[ imageIndex ], 0, nullptr );
		if ( m_iDrawType == DT_DrawElements )
			vkCmdDrawIndexed( m_vkSecondaryCommandBuffers[ imageIndex ], static_cast< uint32_t >( m_Indices.size() ), 1, 0, 0, 0 );
		else
			vkCmdDraw( m_vkSecondaryCommandBuffers[ imageIndex ], static_cast< uint32_t >( m_Vertices.size() ), 1, 0, 0 );
	vkEndCommandBuffer( m_vkSecondaryCommandBuffers[ imageIndex ] );

	return m_vkSecondaryCommandBuffers[ imageIndex ];
}

void MeshVK::createVertexBuffer()
{
	if ( m_Vertices.size() == 0 )
		return;

	VkDeviceSize bufferSize = sizeof( m_Vertices[ 0 ] ) * m_Vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;

	vkMapMemory( vulkan().device, stagingBufferMemory, 0, bufferSize, 0, &pData );
		std::memcpy( pData, m_Vertices.data(), static_cast< size_t >( bufferSize ) );
	vkUnmapMemory( vulkan().device, stagingBufferMemory );

	VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vkVertexBuffer, m_vkVertexBufferMemory );
	VulkanApp().copyBuffer( stagingBuffer, m_vkVertexBuffer, bufferSize );

	vkDestroyBuffer( vulkan().device, stagingBuffer, nullptr );
	vkFreeMemory( vulkan().device, stagingBufferMemory, nullptr );
}

void MeshVK::createIndexBuffer()
{
	if ( m_Indices.size() == 0 )
		return;

	VkDeviceSize bufferSize = sizeof( m_Indices[ 0 ] ) * m_Indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;

	vkMapMemory( vulkan().device, stagingBufferMemory, 0, bufferSize, 0, &pData );
		std::memcpy( pData, m_Indices.data(), static_cast< size_t >( bufferSize ) );
	vkUnmapMemory( vulkan().device, stagingBufferMemory );

	VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vkIndexBuffer, m_vkIndexBufferMemory );
	VulkanApp().copyBuffer( stagingBuffer, m_vkIndexBuffer, bufferSize );

	vkDestroyBuffer( vulkan().device, stagingBuffer, nullptr );
	vkFreeMemory( vulkan().device, stagingBufferMemory, nullptr );
}

void MeshVK::createCommandPool()
{
	auto queueFamilyIndices = VulkanApp().findQueueFamilies( vulkan().physicalDevice );

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if ( vkCreateCommandPool( vulkan().device, &poolInfo, nullptr, &m_vkCommandPool ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create command pool." );
}

void MeshVK::allocateSecondaryCommandBuffers()
{
	m_vkSecondaryCommandBuffers.resize( vulkan().swapChainFramebuffers.size(), VK_NULL_HANDLE );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = static_cast< uint32_t >( vulkan().swapChainFramebuffers.size() );

	if ( vkAllocateCommandBuffers( vulkan().device, &allocInfo, m_vkSecondaryCommandBuffers.data() ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create command buffers." );
}