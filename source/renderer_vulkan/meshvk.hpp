#ifndef MESHVK_HPP
#define MESHVK_HPP

#include "imesh.hpp"
#include "vertex.hpp"

#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan_interface.hpp"

class MaterialVK;
class ModelVK;

class MeshVK : public vkApp::CVulkanInterface
{
	typedef std::vector< Vertex > Vertices;
	typedef std::vector< uint32_t > Indices;

public:
	MeshVK( const Vertices &vertices, const Indices &indices, MaterialVK *pMaterial, ModelVK *pOwningModel = nullptr );
	virtual ~MeshVK();

	void Shutdown();

	void Draw( VkCommandBuffer commandBuffer, const uint32_t &imageIndex );
	const VkCommandBuffer &RecordSecondaryCommandBuffers( VkCommandBufferInheritanceInfo inheritanceInfo, const uint32_t &imageIndex );

	MaterialVK *GetMaterial() const { return m_pMaterial; }

	const Indices &GetIndices() const { return m_Indices; }
	const Vertices &GetVertices() const { return m_Vertices; }

	const ModelVK *GetOwningModel() const { return m_pOwningModel; }

private:
	enum DrawType
	{
		DT_DrawArrays = 0,
		DT_DrawElements
	};

	void createVertexBuffer();
	void createIndexBuffer();

	void createCommandPool();

	void allocateSecondaryCommandBuffers();

	DrawType m_iDrawType;

	Vertices m_Vertices;
	Indices m_Indices;

	VkCommandPool m_vkCommandPool = VK_NULL_HANDLE;
	std::vector< VkCommandBuffer > m_vkSecondaryCommandBuffers;

	VkBuffer m_vkVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vkVertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_vkIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vkIndexBufferMemory = VK_NULL_HANDLE;

	MaterialVK *m_pMaterial = nullptr;
	ModelVK *m_pOwningModel = nullptr;
};

#endif // MESHVK_HPP