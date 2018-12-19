#include "meshvk.hpp"
#include "renderer_vulkan.hpp"
#include "materialvk.hpp"
#include "shadervk.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

MeshVK::MeshVK( Vertices vertices, Indices indices, MaterialVK *pMaterial )
{
	m_Vertices = vertices;
	m_Indices = indices;
	m_pMaterial = pMaterial;

	if ( m_Indices.size() > 0 )
	{
		m_iDrawType = DT_DrawElements;
	}
	else
	{
		m_iDrawType = DT_DrawArrays;
	}

	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createCommandBuffers();
}

MeshVK::~MeshVK()
{
	Shutdown();
}

void MeshVK::Shutdown()
{
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

void MeshVK::Draw( const uint32_t &imageIndex )
{
	// TODO: Update Uniform Buffers

}

void MeshVK::createCommandPool()
{
	auto queueFamilyIndices = VulkanApp().findQueueFamilies( vulkan().physicalDevice );

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0; // Optional

	if ( vkCreateCommandPool( vulkan().device, &poolInfo, nullptr, &m_vkCommandPool ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create command pool." );
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

void MeshVK::createCommandBuffers()
{
	ShaderVK *pShader = m_pMaterial->GetShader();

	m_vkCommandBuffers.resize( vulkan().swapChainFramebuffers.size(), VK_NULL_HANDLE );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast< uint32_t >( m_vkCommandBuffers.size() );

	if ( vkAllocateCommandBuffers( vulkan().device, &allocInfo, m_vkCommandBuffers.data() ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create command buffers." );

	for ( size_t i = 0; i < m_vkCommandBuffers.size(); ++i )
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if ( vkBeginCommandBuffer( m_vkCommandBuffers[ i ], &beginInfo ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pShader->m_vkRenderPass;
		renderPassInfo.framebuffer = vulkan().swapChainFramebuffers[ i ];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vulkan().swapChainExtent;

		std::array< VkClearValue, 2 > clearValues;
		clearValues[ 0 ].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[ 1 ].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size() );
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass( m_vkCommandBuffers[ i ], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
			vkCmdBindPipeline( m_vkCommandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->m_vkGraphicsPipeline );
			VkBuffer vertexBuffers[] = { m_vkVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers( m_vkCommandBuffers[ i ], 0, 1, vertexBuffers, offsets );
			if ( m_iDrawType == DT_DrawElements )
				vkCmdBindIndexBuffer( m_vkCommandBuffers[ i ], m_vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );
			vkCmdBindDescriptorSets( m_vkCommandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->m_vkPipelineLayout, 0, 1, &m_pMaterial->m_vkDescriptorSets[ i ], 0, nullptr );
			if ( m_iDrawType == DT_DrawElements)
				vkCmdDrawIndexed( m_vkCommandBuffers[ i ], static_cast< uint32_t >( m_Indices.size() ), 1, 0, 0, 0 );
			else
				vkCmdDraw( m_vkCommandBuffers[ i ], static_cast< uint32_t >( m_Vertices.size() ), 1, 0, 0 );
		vkCmdEndRenderPass( m_vkCommandBuffers[ i ] );

		if ( vkEndCommandBuffer( m_vkCommandBuffers[ i ] ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );
	}
}