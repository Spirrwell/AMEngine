#ifndef MESHVK_HPP
#define MESHVK_HPP

#include "imesh.hpp"
#include "vertex.hpp"
#include "vulkan_helpers.hpp"

#include <vector>

#include "vulkan/vulkan.hpp"

class MeshVK : public IMesh
{
	typedef std::vector< Vertex > Vertices;
	typedef std::vector< uint32_t > Indices;

public:
	MeshVK( Vertices vertices, Indices indices );
	virtual ~MeshVK();

	void Draw() override;
	void SetModelMatrix( Matrix4f modelMatrix ) override {}

	const Indices &GetIndices() { return m_Indices; }
	const Vertices &GetVertices() { return m_Vertices; }

private:
	enum DrawType
	{
		DT_DrawArrays = 0,
		DT_DrawElements
	};

	vkApp::VulkanApp &VulkanApp() { return m_vkApp; }

	void createVertexBuffer();
	void createIndexBuffer();

	DrawType m_iDrawType;

	Vertices m_Vertices;
	Indices m_Indices;

	vkApp::VulkanApp &m_vkApp;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
};

#endif // MESHVK_HPP