#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

#include "renderer_opengl.hpp"
#include "engine/iengine.hpp"
#include "factory/ifactory.hpp"
#include "interface.hpp"
#include "shadersystem/ishadermanager.hpp"
#include "camera.hpp"
#include "game/client/iclient.hpp"
#include "input/iinput.hpp"
#include "materialsystem/imaterial.hpp"
#include "materialsystem/imaterialsystem.hpp"
#include "texturegl.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IEngine *g_pEngine = nullptr;
IShaderManager *g_pShaderManager = nullptr;
IInput *g_pInput = nullptr;
IMaterialSystem *g_pMaterialSystem = nullptr;


RendererGL::RendererGL()
{
	m_pMainWindow = nullptr;
	m_hGLContext = nullptr;
	//m_pMesh = nullptr;
	m_pModel = nullptr;
	//m_pFloorMesh = nullptr;
	m_pSkybox = nullptr;
}

RendererGL::~RendererGL()
{
}

bool RendererGL::Init()
{
	g_pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );

	if ( g_pEngine == nullptr )
	{
		printf( "Renderer failed to get engine interface!\n" );
		return false;
	}

	config cfg = g_pEngine->GetConfig();
	Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

	if ( cfg.windowConfig.fullscreen )
		windowFlags |= SDL_WINDOW_FULLSCREEN;

	m_pMainWindow = SDL_CreateWindow
	(
		"AMEngine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		cfg.windowConfig.resolution_width,
		cfg.windowConfig.resolution_height,
		windowFlags
	);

	if ( m_pMainWindow == nullptr )
	{
		printf( "Renderer: %s\n", SDL_GetError() );
		return false;
	}

	m_hGLContext = SDL_GL_CreateContext( m_pMainWindow );

	if ( !m_hGLContext )
	{
		printf( "%s\n", SDL_GetError() );
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum eError = glewInit();

	if ( eError != GLEW_OK )
	{
		printf( "%s\n", glewGetErrorString( eError ) );
		return false;
	}

	glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );

	glFrontFace( GL_CCW );
	glCullFace( GL_FRONT );
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_DEPTH_CLAMP );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	SDL_GL_SetSwapInterval( 0 );

	SDL_SetWindowGrab( m_pMainWindow, SDL_TRUE );
	SDL_SetRelativeMouseMode( SDL_TRUE );

	g_pInput = ( IInput* )GetFactory()->GetInterface( INPUT_INTERFACE_VERSION );

	if ( g_pInput == nullptr )
	{
		printf( "Failed to get input interface!\n" );
		return false;
	}

	if ( !GetFactory()->LoadModule( "bin/shadergl" + string( DLL_EXTENSION ) ) )
    {
        printf( "Renderer failed to get shadergl module\n" );
		return false;
    }

	g_pShaderManager = ( IShaderManager* )GetFactory()->GetInterface( SHADERMANAGER_INTERFACE );

	if ( !g_pShaderManager || !g_pShaderManager->Init() )
		return false;

	if ( !GetFactory()->LoadModule( "bin/materialsystem" + string( DLL_EXTENSION ) ) )
    {
        printf( "Renderer failed to get materialsystem module\n" );
		return false;
    }

	g_pMaterialSystem = ( IMaterialSystem* )GetFactory()->GetInterface( MATERIALSYSTEM_INTERFACE_VERSION );

	if ( !g_pMaterialSystem || !g_pMaterialSystem->Init() )
		return false;

	std::vector < Vertex > vertices =
	{
		Vertex( Vector3f( -0.5f, -0.5f, 0.0f ) ),
		Vertex( Vector3f( 0.0f, 0.5f, 0.0f ) ),
		Vertex( Vector3f( 0.5f, -0.5f, 0.0f ) ),
	};
	std::vector < unsigned int > indices =
	{
		0, 1, 2
	};

	float fieldDepth = 5.0f;
	float fieldWidth = 5.0f;

	std::vector < Vertex > floorVertices =
	{
		Vertex( Vector3f( -fieldWidth, -3.0f, -fieldDepth ) ),
		Vertex( Vector3f( -fieldWidth, -3.0f, fieldDepth * 3.0f ) ),
		Vertex( Vector3f( fieldWidth * 3.0f, -3.0f, -fieldDepth ) ),
		Vertex( Vector3f( fieldWidth * 3.0f, -3.0f, fieldDepth * 3.0f ) )
	};

	std::vector < unsigned int > floorIndices =
	{
		0, 1, 2,
		2, 1, 3,
	};

	m_pModel = new ModelGL( string( GAME_DIR ) + "models/chalet_mat.amdl" );
	m_pSkybox = new Skybox;

	//TextureGL *pTexture = new TextureGL( string( GAME_DIR ) + "models/cube.fbm/test2.png" );

	//m_pMesh = new MeshGL( vertices, indices, m_pShaders[ 0 ] );
	//m_pMesh = new MeshGL( realModelVerts, realModelIndicies, m_pShaders[ 0 ] );
	//m_pMesh = new MeshGL( meshVertices, meshIndices, pMat );
	//m_pFloorMesh = new MeshGL( floorVertices, floorIndices, pMat );

	//m_pFloorMesh->transform.SetPitch( glm::radians( 180.0f ) );

	return true;
}

void RendererGL::PostInit()
{
	for ( auto &viewport : m_pViewPorts )
		viewport->InitPerspectiveViewPort( glm::radians( 70.0f ), g_pEngine->GetAspectRatio(), 0.01f, 1000.0f, new Camera( Vector3f( 0.0f, 0.0f, -1.0f ) ) );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void RendererGL::Shutdown()
{
	//for ( auto &pShader : m_pShaders )
		//pShader->Shutdown();

	for ( auto &pViewPort : m_pViewPorts )
        pViewPort->Shutdown();

	if ( m_pSkybox )
		delete m_pSkybox;

	if ( m_pModel )
		delete m_pModel;

	//if ( m_pFloorMesh )
		//delete m_pFloorMesh;

	if ( m_hGLContext != nullptr )
	{
		SDL_GL_DeleteContext( m_hGLContext );
		m_hGLContext = nullptr;
	}

	if ( m_pMainWindow != nullptr )
	{
		SDL_DestroyWindow( m_pMainWindow );
		m_pMainWindow = nullptr;
	}
}

void RendererGL::DrawScene()
{
	//for ( unsigned int i = 0; i < m_pViewPorts.Count(); i++ )
		//m_pViewPorts[ i ]->UpdateViewPort();

	m_pModel->transform.SetPosition( Vector3f( 0.0f, 0.0f, 0.0f ) );
	m_pModel->transform.UpdateModel();

	m_pModel->Draw();

	if ( m_pSkybox->IsValid() )
		m_pSkybox->Draw();

	//m_pMesh->GetShader()->SetColor( Vector4f( 0.0f, 1.0f, 0.0f, 1.0f ) );
	//m_pMesh->Draw();

	//m_pFloorMesh->GetShader()->SetColor( Vector4f( 1.0f, 0.0f, 0.0f, 1.0f ) );
	//m_pFloorMesh->Draw();
}

void RendererGL::Clear()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void RendererGL::Swap()
{
	SDL_GL_SwapWindow( m_pMainWindow );
}

ITexture *RendererGL::CreateTexture( const string &textureName, bool bCubeMap /*= false*/ )
{
	if ( bCubeMap )
	{
		std::vector < string > faces =
		{
			string( GAME_DIR ) + textureName + "_right.jpg",
			string( GAME_DIR ) + textureName + "_left.jpg",
			string( GAME_DIR ) + textureName + "_top.jpg",
			string( GAME_DIR ) + textureName + "_bottom.jpg",
			string( GAME_DIR ) + textureName + "_back.jpg",
			string( GAME_DIR ) + textureName + "_front.jpg"
		};

		return new TextureGL( faces );
	}

	return new TextureGL( string( GAME_DIR ) + textureName );
}

unsigned int RendererGL::GetShaderCount()
{
	return ( unsigned int )m_pShaders.size();
}

IBaseShader *RendererGL::GetShader( unsigned int iShaderIndex )
{
	assert( iShaderIndex < m_pShaders.size() );
	return m_pShaders[ iShaderIndex ];
}

IBaseShader *RendererGL::GetShader( const string &shaderName )
{
	for ( IBaseShader *& pShader : m_pShaders )
		if ( pShader->GetName() == shaderName )
			return pShader;

	return nullptr;
}

unsigned int RendererGL::AddShader( IBaseShader *pShader )
{
	assert( pShader );
	return ( unsigned int )std::distance( m_pShaders.begin(), m_pShaders.insert( m_pShaders.end(), pShader ) );
}

void RendererGL::AddViewPort( IViewPort *pViewPort )
{
	m_pViewPorts.push_back( pViewPort );
}

static DLLInterface< IRenderer, RendererGL > s_Renderer( RENDERER_INTERFACE );
RendererGL &GetGLRenderer_Internal() { return *s_Renderer.GetInternal(); }
