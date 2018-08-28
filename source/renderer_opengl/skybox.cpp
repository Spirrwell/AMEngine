#include "skybox.hpp"
#include "renderer_opengl.hpp"
#include "platform.hpp"
#include "texturegl.hpp"
#include "materialsystem/imaterialsystem.hpp"
#include "materialsystem/imaterial.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern RendererGL *GetGLRenderer_Internal();
extern IMaterialSystem *g_pMaterialSystem;

static float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

Skybox::Skybox()
{
	m_pMaterial = nullptr;
	m_pShader = GetGLRenderer_Internal()->GetShader( "skyShader" );

	if ( m_pShader )
	{
		m_pMaterial = g_pMaterialSystem->CreateMaterial( "materials/skybox/default_sky.amat" );

		glGenVertexArrays( 1, &m_VertexArrayObject );
		glGenBuffers( 1, &m_VertexArrayBuffer );

		glBindVertexArray( m_VertexArrayObject );
		glBindBuffer( GL_ARRAY_BUFFER, m_VertexArrayBuffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices ), &skyboxVertices, GL_STATIC_DRAW );

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
	}
}

Skybox::~Skybox()
{
	if ( m_pMaterial != nullptr )
		delete m_pMaterial;
}

void Skybox::Draw()
{
	glDepthFunc( GL_LEQUAL );

	IShader *pShader = m_pShader->GetIShader();

	pShader->BindShader();
	pShader->Update( m_pMaterial );

	glBindVertexArray( m_VertexArrayObject );
	m_pMaterial->Bind();

	glDrawArrays( GL_TRIANGLES, 0, sizeof( skyboxVertices ) );

	glBindVertexArray( 0 );
	glDepthFunc( GL_LESS );
}
