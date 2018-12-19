#ifndef MESHVK_HPP
#define MESHVK_HPP

#include "imesh.hpp"
#include "vertex.hpp"

#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan_interface.hpp"

class MaterialVK;

class MeshVK : public vkApp::CVulkanInterface
{
	typedef std::vector< Vertex > Vertices;
	typedef std::vector< uint32_t > Indices;

public:
	MeshVK( Vertices vertices, Indices indices, MaterialVK *pMaterial );
	virtual ~MeshVK();

	void Shutdown();

	void Draw( const uint32_t &imageIndex );

	const Indices &GetIndices() { return m_Indices; }
	const Vertices &GetVertices() { return m_Vertices; }

private:
	enum DrawType
	{
		DT_DrawArrays = 0,
		DT_DrawElements
	};

	void createCommandPool();
	void createVertexBuffer();
	void createIndexBuffer();
	void createCommandBuffers();

	DrawType m_iDrawType;

	Vertices m_Vertices;
	Indices m_Indices;

	VkCommandPool m_vkCommandPool = VK_NULL_HANDLE;
	std::vector< VkCommandBuffer > m_vkCommandBuffers;

	VkBuffer m_vkVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vkVertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_vkIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vkIndexBufferMemory = VK_NULL_HANDLE;

	MaterialVK *m_pMaterial = nullptr;
};

#endif // MESHVK_HPP