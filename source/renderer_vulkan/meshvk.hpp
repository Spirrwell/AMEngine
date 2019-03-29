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

	void Draw( vk::CommandBuffer &commandBuffer, const uint32_t &imageIndex );
	const vk::CommandBuffer &RecordSecondaryCommandBuffers( vk::CommandBufferInheritanceInfo inheritanceInfo, const uint32_t &imageIndex );

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

	vk::CommandPool m_vkCommandPool;
	std::vector< vk::CommandBuffer > m_vkSecondaryCommandBuffers;

	vk::Buffer m_vkVertexBuffer;
	vk::DeviceMemory m_vkVertexBufferMemory;

	vk::Buffer m_vkIndexBuffer;
	vk::DeviceMemory m_vkIndexBufferMemory;

	MaterialVK *m_pMaterial = nullptr;
	ModelVK *m_pOwningModel = nullptr;
};

#endif // MESHVK_HPP