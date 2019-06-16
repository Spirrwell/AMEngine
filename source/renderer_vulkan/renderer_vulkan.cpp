#include "renderer_vulkan.hpp"
#include "input/iinput.hpp"
#include "interface.hpp"
#include "factory/ifactory.hpp"
#include "materialsystem/imaterialsystem.hpp"
#include "shadervk.hpp"
#include "camera.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IEngine *g_pEngine = nullptr;
IInput *g_pInput = nullptr;
IMaterialSystem *g_pMaterialSystem = nullptr;

extern Camera g_vkcam;

RendererVulkan::RendererVulkan()
{
	m_pMainWindow = nullptr;
}

bool RendererVulkan::Init()
{
	g_pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );

	if ( g_pEngine == nullptr )
	{
		printf( "Renderer failed to get engine interface!\n" );
		return false;
	}

	const config &cfg = g_pEngine->GetConfig();
	Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;

	if ( cfg.windowConfig.fullscreen )
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	else
		windowFlags |= SDL_WINDOW_RESIZABLE;

	m_pMainWindow = SDL_CreateWindow
	(
		"AMEngine",
		SDL_WINDOWPOS_CENTERED_DISPLAY( 0 ),
		SDL_WINDOWPOS_CENTERED_DISPLAY( 0 ),
		cfg.windowConfig.resolution_width,
		cfg.windowConfig.resolution_height,
		windowFlags
	);

	if ( m_pMainWindow == nullptr )
	{
		printf( "Renderer: %s\n", SDL_GetError() );
		return false;
	}

	// Process events immediately because creating the window can signal a window resize event :/
	GetSDL_Core()->ProcessEvents();

	SDL_SetWindowGrab( m_pMainWindow, SDL_TRUE );
	SDL_SetRelativeMouseMode( SDL_TRUE );

	g_pInput = ( IInput* )GetFactory()->GetInterface( INPUT_INTERFACE_VERSION );

	if ( g_pInput == nullptr )
	{
		printf( "Failed to get input interface!\n" );
		return false;
	}

	/*if ( !GetFactory()->LoadModule( "bin/shadervulkan" + string( DLL_EXTENSION ) ) )
    {
        printf( "Renderer failed to get shadervulkan module\n" );
		return false;
    }*/

	/*g_pShaderManager = ( IShaderManager* )GetFactory()->GetInterface( SHADERMANAGER_INTERFACE );

	if ( !g_pShaderManager || !g_pShaderManager->Init() )
		return false;*/

	if ( !GetFactory()->LoadModule( "bin/materialsystem" + string( DLL_EXTENSION ) ) )
    {
        printf( "Renderer failed to get materialsystem module\n" );
		return false;
    }

	g_pMaterialSystem = ( IMaterialSystem* )GetFactory()->GetInterface( MATERIALSYSTEM_INTERFACE_VERSION );

	if ( !g_pMaterialSystem || !g_pMaterialSystem->Init() )
		return false;

	try
	{
		if ( !VulkanApp().initVulkan() )
			return false;
	}
	catch ( const std::exception &e )
	{
		printf( "%s\n", e.what() );
		return false;
	}


	/*uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

	stprintf( "Extension Count: %u\n", extensionCount );*/

	return true;
}

void RendererVulkan::PostInit()
{

}

void RendererVulkan::Shutdown()
{
	m_vkApp.cleanup();

	if ( m_pMainWindow != nullptr )
	{
		SDL_DestroyWindow( m_pMainWindow );
		m_pMainWindow = nullptr;
	}
}

void RendererVulkan::DrawScene()
{
	m_vkApp.drawFrame();
}

void RendererVulkan::CPUFrame()
{
	g_vkcam.Update();
	g_vkcam.UpdateView();
}

ShaderVK *RendererVulkan::FindShader( const string &shaderName )
{
	const auto &shaders = VulkanApp().GetShaders();

	for ( auto &pShader : shaders )
	{
		if ( pShader->GetShaderName() == shaderName )
			return pShader;
	}

	return nullptr;
}

static DLLInterface< IRenderer, RendererVulkan > s_Renderer( RENDERER_INTERFACE );
RendererVulkan &GetVkRenderer_Internal() { return *s_Renderer.GetInternal(); }