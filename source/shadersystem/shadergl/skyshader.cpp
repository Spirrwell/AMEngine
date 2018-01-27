#include "skyshader.hpp"

/*const GLchar *pszVSUBONames[2] =
{
	"MatrixBlock.view",
	"MatrixBlock.projection"
};*/

void SkyShader::Initialize()
{
	ShaderGL::Initialize();
	LinkAndValidate();

	//AddUniform( "view" );
	//AddUniform( "projection" );

	CreateUniformBlock( "MatrixBlock" );

	AddUniformToBlock( "MatrixBlock", "view" );
	AddUniformToBlock( "MatrixBlock", "projection" );

	/*vsBlockIndex = glGetUniformBlockIndex( m_hProgram, "MatrixBlock" );
	vsBlockSize = 0;

	glGetActiveUniformBlockiv( m_hProgram, vsBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &vsBlockSize );

	GLuint vsIndices[ 2 ];
	glGetUniformIndices( m_hProgram, 2, pszVSUBONames, vsIndices );

	GLint vsOffsets[ 2 ];
	glGetActiveUniformsiv( m_hProgram, 2, vsIndices, GL_UNIFORM_OFFSET, vsOffsets );

	for ( size_t i = 0; i < 2; i++ )
	{
		std::string uniformName = std::string( pszVSUBONames[i] );
		m_mapVSUniformOffsets[ uniformName ] = vsOffsets[i];
	}

	//vsBlockBuffer = ( GLubyte* )malloc( vsBlockSize );

	glGenBuffers( 1, &vsUBOHandle );
	glBindBuffer( GL_UNIFORM_BUFFER, vsUBOHandle );
	glBufferData( GL_UNIFORM_BUFFER, vsBlockSize, nullptr, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, vsBlockIndex, vsUBOHandle );*/

	AddUniform( "skybox" ); // binding 0

	m_vMaterialParameters.push_back( MaterialParameter_t { "skybox", MATP_SKYTEXTURE } );
}

void SkyShader::Shutdown()
{
	ShaderGL::Shutdown();

	//glDeleteBuffers( 1, &vsUBOHandle );

	//free( vsBlockBuffer );
}

void SkyShader::Update( IMaterial *pMaterial )
{
	ITexture *pSkybox = pMaterial->GetTexture( "skybox" );
	pSkybox->Bind( 0, GL_TEXTURE_CUBE_MAP );

	Matrix4f view = Matrix4f( Matrix3f( g_pRenderer->GetViewPort()->GetViewMatrix() ) );
	Matrix4f projection = g_pRenderer->GetViewPort()->GetProjection();

	BindUBO( "MatrixBlock" );

	SetMatrix4f( "view", view );
	SetMatrix4f( "projection", projection );

	UnbindUBO();
	//glBindBuffer( GL_UNIFORM_BUFFER, vsUBOHandle );

	//glBufferSubData( GL_UNIFORM_BUFFER, m_mapVSUniformOffsets[ "MatrixBlock.view" ], sizeof( view ), &view[0] );
	//glBufferSubData( GL_UNIFORM_BUFFER, m_mapVSUniformOffsets[ "MatrixBlock.projection" ], sizeof( projection ), &projection[0] );

	//SetMatrix4f( "view", Matrix4f( Matrix3f( g_pRenderer->GetViewPort()->GetViewMatrix() ) ) );
	//SetMatrix4f( "projection", g_pRenderer->GetViewPort()->GetProjection() );
}
