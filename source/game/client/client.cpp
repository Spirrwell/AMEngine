#include "client.hpp"
#include "engine/iengine.hpp"
#include "interface.hpp"
#include "renderer/irenderer.hpp"
#include "viewport.hpp"
#include "gameobjectfactory.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IEngine *g_pEngine = nullptr;
IRenderer *g_pRenderer = nullptr;

bool ClientDLL::Init()
{
	g_pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );

	if ( g_pEngine == nullptr )
	{
		printf( "Failed to get engine interface!\n" );
		return false;
	}

	g_pRenderer = ( IRenderer* )GetFactory()->GetInterface( RENDERER_INTERFACE_OPENGL );

	if ( g_pRenderer == nullptr )
	{
		printf( "Failed to get renderer interface!\n" );
		return false;
	}

	static ViewPort s_MainViewPort;

	return true;
}

IGameObjectFactory *ClientDLL::GetGameObjectFactory()
{
	return ::GetGameObjectFactory();
}

static DLLInterface< IClientDLL, ClientDLL > s_ClientDLL( CLIENT_INTERFACE_VERSION );