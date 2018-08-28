#include "meshgl.hpp"
#include "renderer_opengl.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern RendererGL *GetGLRenderer_Internal();

MeshGL::MeshGL( std::vector < Vertex > Vertices, std::vector < unsigned int > Indices, IMaterial *pMaterial )
{
	m_pMaterial = pMaterial;
	m_Vertices = Vertices;
	m_iIndices = Indices;

	if ( m_iIndices.size() > 0 )
	{
		m_iDrawType = DT_DrawElements;
		m_iDrawCount = ( unsigned int )m_iIndices.size();
	}
	else
	{
		m_iDrawType = DT_DrawArrays;
		m_iDrawCount = ( unsigned int )m_Vertices.size();
	}

	glGenVertexArrays( 1, &m_VertexArrayObject );
	glBindVertexArray( m_VertexArrayObject );

	//std::vector < Vector3f > positions ( m_Vertices.size() );

	//for ( unsigned int i = 0; i < m_Vertices.size(); i++ )
		//positions[i] = m_Vertices[i].GetPosition();

	std::vector < GLfloat > positions;
	std::vector < GLfloat > texCoords;
	std::vector < GLfloat > normals;

	for ( Vertex &vertex : m_Vertices )
	{
		positions.push_back( vertex.GetPosition().x );
		positions.push_back( vertex.GetPosition().y );
		positions.push_back( vertex.GetPosition().z );

		texCoords.push_back( vertex.GetTexCoord().x );
		texCoords.push_back( vertex.GetTexCoord().y );

		normals.push_back( vertex.GetNormal().x );
		normals.push_back( vertex.GetNormal().y );
		normals.push_back( vertex.GetNormal().z );
	}

	glGenBuffers( NUM_BUFFERS, m_VertexArrayBuffers );

	glBindBuffer( GL_ARRAY_BUFFER, m_VertexArrayBuffers[POSITION_VB] );
	glBufferData( GL_ARRAY_BUFFER, positions.size() * sizeof( positions[0] ), positions.data(), GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindBuffer( GL_ARRAY_BUFFER, m_VertexArrayBuffers[TEXCOORD_VB] );
	glBufferData( GL_ARRAY_BUFFER, texCoords.size() * sizeof( texCoords[0] ), texCoords.data(), GL_STATIC_DRAW );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindBuffer( GL_ARRAY_BUFFER, m_VertexArrayBuffers[NORMAL_VB] );
	glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof( normals[0] ), normals.data(), GL_STATIC_DRAW );

	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	if ( m_iDrawType == DT_DrawElements )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_VertexArrayBuffers[INDEX_VB] );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof ( m_iIndices[0] ) * m_iIndices.size(), m_iIndices.data(), GL_STATIC_DRAW );
	}

	glBindVertexArray( 0 );
}

MeshGL::~MeshGL()
{
	glDeleteBuffers( NUM_BUFFERS, m_VertexArrayBuffers );
	glDeleteVertexArrays( 1, &m_VertexArrayObject );

	if ( m_pMaterial != nullptr )
		delete m_pMaterial;
}

void MeshGL::Draw()
{
	m_pMaterial->Bind();

	IShader *pShader = GetShader()->GetIShader();

	pShader->BindUBO( "UBO" );

	pShader->SetMatrix4f( "model", m_mat4Model );
	pShader->SetMatrix4f( "normalMatrix", glm::transpose( glm::inverse( GetGLRenderer_Internal()->GetViewPort()->GetViewMatrix() * m_mat4Model ) ) );

	pShader->UnbindUBO();

	glBindVertexArray( m_VertexArrayObject );

	// Should maybe move drawing to function pointer instead of check "if" every frame. Minor performance thing
	if ( m_iDrawType == DT_DrawArrays )
		glDrawArrays( GL_TRIANGLES, 0, m_iDrawCount );
	else if ( m_iDrawType == DT_DrawElements )
		glDrawElements( GL_TRIANGLES, m_iDrawCount, GL_UNSIGNED_INT, nullptr );


	glBindVertexArray( 0 );
}
