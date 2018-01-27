#ifndef MESHGL_HPP
#define MESHGL_HPP

#include "imesh.hpp"

#include "GL/glew.h"
#include "SDL_opengl.h"

#include "amlib/transform.hpp"
#include "shadersystem/ishader.hpp"

#include "materialsystem/imaterial.hpp"

#include <vector>

// TODO: Give this its own set of files
class Vertex
{
public:
	Vertex()
	{
		m_vPostion = Vector3f();
		m_vTexCoord = Vector2f();
		m_vNormal = Vector3f();
	}
	Vertex( Vector3f vPosition )
	{
		m_vPostion = vPosition;
		m_vTexCoord = Vector2f();
		m_vNormal = Vector3f();
	}
	Vertex( Vector3f vPosition, Vector2f vTexCoord )
	{
		m_vPostion = vPosition;
		m_vTexCoord = vTexCoord;
		m_vNormal = Vector3f();
	}

	Vertex( Vector3f vPosition, Vector2f vTexCoord, Vector3f vNormal )
	{
		m_vPostion = vPosition;
		m_vTexCoord = vTexCoord;
		m_vNormal = vNormal;
	}

	const Vector3f &GetPosition() { return m_vPostion; }
	const Vector2f &GetTexCoord() { return m_vTexCoord; }
	const Vector3f &GetNormal() { return m_vNormal; }

private:
	Vector3f m_vPostion;
	Vector2f m_vTexCoord;
	Vector3f m_vNormal;
};

class MeshGL : public IMesh
{
public:
	MeshGL( std::vector < Vertex > Vertices, std::vector < unsigned int > Indices, IMaterial *pMaterial );
	virtual ~MeshGL();

	virtual void Draw();
	virtual void SetModelMatrix( Matrix4f modelMatrix ) { m_mat4Model = modelMatrix; }

	IBaseShader *GetShader() { return m_pMaterial->GetShader(); }

	uint32_t GetVertexCount() { return ( uint32_t )m_Vertices.size(); }
	uint32_t GetIndexCount() { return ( uint32_t )m_iIndices.size(); }

	std::vector < Vertex > GetVertices() { return m_Vertices; }
	std::vector < unsigned int > GetIndicies() { return m_iIndices; }

	IMaterial *GetMaterial() { return m_pMaterial; }

private:
	enum
	{
		POSITION_VB,
		TEXCOORD_VB,
		NORMAL_VB,

		INDEX_VB,

		NUM_BUFFERS
	};

	enum DrawType
	{
		DT_DrawArrays = 0,
		DT_DrawElements
	};

	std::vector < Vertex > m_Vertices;
	std::vector < unsigned int > m_iIndices;

	unsigned int m_iDrawCount;

	GLuint m_VertexArrayObject;
	GLuint m_VertexArrayBuffers[NUM_BUFFERS];

	IMaterial *m_pMaterial;

	DrawType m_iDrawType;
	Matrix4f m_mat4Model;
};

#endif // MESHGL_HPP