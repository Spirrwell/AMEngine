#include "shadermanagergl.hpp"
#include "interface.hpp"
#include "renderer/irenderer.hpp"
#include "shadergl.hpp"

IRenderer *g_pRenderer = nullptr;

bool ShaderManagerGL::Init()
{
	g_pRenderer = ( IRenderer* )GetFactory()->GetInterface( RENDERER_INTERFACE_OPENGL );

	if ( g_pRenderer == nullptr )
    {
        printf( "Failed to get GL Renderer Interface!\n" );
		return false;
    }

	if ( !GetFactory()->LoadModule( "bin/standard_shaders" + std::string( DLL_EXTENSION ) ) )
	{
		printf( "Failed to load standard_shaders module!\n" );
		return false;
	}

	for ( IBaseShader *& pShader : m_pShaders )
		pShader->InitShaderParams();

	return true;
}

void ShaderManagerGL::InserShader( IBaseShader *pShader )
{
	assert( pShader != nullptr );
	m_pShaders.push_back( pShader );
}

IShader *ShaderManagerGL::CreateShaderObject( const std::string &strShaderName )
{
	ShaderGL *pShader = new ShaderGL( strShaderName );

	pShader->Initialize();
	pShader->LinkAndValidate();

	return pShader;
}

static DLLInterface < IShaderManager, ShaderManagerGL > s_ShaderManager( SHADERMANAGER_INTERFACE_GL );