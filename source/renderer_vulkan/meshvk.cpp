#include "meshvk.hpp"
#include "renderer_vulkan.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

MeshVK::MeshVK( Vertices vertices, Indices indices )
{
	m_Vertices = vertices;
	m_Indices = indices;

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
}

MeshVK::~MeshVK()
{
	Shutdown();
}

void MeshVK::Shutdown()
{
	if ( indexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( vulkan().device, indexBuffer, nullptr );
		indexBuffer = VK_NULL_HANDLE;
	}

	if ( indexBufferMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( vulkan().device, indexBufferMemory, nullptr );
		indexBufferMemory = VK_NULL_HANDLE;
	}

	if ( vertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( vulkan().device, vertexBuffer, nullptr );
		vertexBuffer = VK_NULL_HANDLE;
	}

	if ( vertexBufferMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( vulkan().device, vertexBufferMemory, nullptr );
		vertexBufferMemory = VK_NULL_HANDLE;
	}
}

void MeshVK::Draw()
{

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

	VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory );
	VulkanApp().copyBuffer( stagingBuffer, vertexBuffer, bufferSize );

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

	VulkanApp().createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory );
	VulkanApp().copyBuffer( stagingBuffer, indexBuffer, bufferSize );

	vkDestroyBuffer( vulkan().device, stagingBuffer, nullptr );
	vkFreeMemory( vulkan().device, stagingBufferMemory, nullptr );
}