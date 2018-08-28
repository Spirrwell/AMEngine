#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

#include "engine.hpp"
#include "renderer/irenderer.hpp"
#include "input/iinput.hpp"
#include "game/client/iclient.hpp"
#include "game/server/iserver.hpp"
#include "enet/enet.h"
#include "engineserver.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IRenderer *g_pRenderer = nullptr;
IInput *g_pInput = nullptr;
IClientDLL *g_pClientDLL = nullptr;
IServerDLL *g_pServerDLL = nullptr;

IGameObjectFactory *g_pServerGameObjectFactory = nullptr;
IGameObjectFactory *g_pClientGameObjectFactory = nullptr;

enum
{
	CFG_RESOLUTION_WIDTH = 0,
	CFG_RESOLUTION_HEIGHT,
	CFG_FULLSCREEN
};

// TODO: Come up with a cleaner way to do this
static const string g_strConfigVariables[] =
{
	"resolution_width",
	"resolution_height",
	"fullscreen"
};

Engine::Engine()
{
	m_bSDLInitialized = false;
	m_bNetworkInitialized = false;

	m_CurrentTime = std::chrono::high_resolution_clock::now();
}

Engine::~Engine()
{
}

bool Engine::Init()
{
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		printf( "%s\n", SDL_GetError() );
		return false;
	}

	m_bSDLInitialized = true;

	if ( !GetFactory()->LoadModule( "bin/input" + string( DLL_EXTENSION ) ) )
	{
		printf( "Failed to load input module!\n" );
		return false;
	}

	g_pInput = ( IInput* )GetFactory()->GetInterface( INPUT_INTERFACE_VERSION );

	if ( g_pInput == nullptr )
	{
		printf( "Failed to get input interface!\n" );
		return false;
	}

	if ( !g_pInput->Init() )
		return false;

	std::ifstream config;
	config.open( string( GAME_DIR ) + "cfg/config.cfg", std::ifstream::in );

	if ( config.is_open() )
	{
		string line;
		while ( !config.eof() )
		{
			std::getline( config, line );
			std::vector < string > tokens;
			std::istringstream ss( line );
			string token;
			while ( ss >> token )
				tokens.push_back( token );

			// Should only be two tokens per line in our config file for now
			if ( tokens.size() == 2 )
			{
				if ( tokens[ 0 ] == g_strConfigVariables[ CFG_RESOLUTION_WIDTH ] )
					m_Config.windowConfig.resolution_width = std::stoi( tokens[ 1 ] );
				else if ( tokens[ 0 ] == g_strConfigVariables[ CFG_RESOLUTION_HEIGHT ] )
					m_Config.windowConfig.resolution_height = std::stoi( tokens[ 1 ] );
				else if ( tokens[ 0 ] == g_strConfigVariables[ CFG_FULLSCREEN ] )
					m_Config.windowConfig.fullscreen = ( std::stoi( tokens[ 1 ] ) != 0 );
			}
		}
	}

	printf( "Width: %d\nHeight: %d\n", m_Config.windowConfig.resolution_width, m_Config.windowConfig.resolution_height );
	printf( "Fullscreen: %d\n", m_Config.windowConfig.fullscreen );

	m_flAspectRatio = ( ( float )m_Config.windowConfig.resolution_width / ( float )m_Config.windowConfig.resolution_height );

	// Use OpenGL explicitly for now
	if ( !GetFactory()->LoadModule( "bin/renderer_opengl" + string( DLL_EXTENSION ) ) )
	{
		printf( "Failed to load renderer module!\n" );
		return false;
	}

	g_pRenderer = ( IRenderer* )GetFactory()->GetInterface( RENDERER_INTERFACE_OPENGL );

	if ( g_pRenderer == nullptr )
	{
		printf( "Failed to get renderer interface!\n" );
		return false;
	}

	if ( !g_pRenderer->Init() )
		return false;

	if ( !GetFactory()->LoadModule( "bin/client" + string( DLL_EXTENSION ) ) )
		return false;

	g_pClientDLL = ( IClientDLL* )GetFactory()->GetInterface( CLIENT_INTERFACE_VERSION );

	if ( g_pClientDLL == nullptr )
	{
		printf( "Failed to get client interface!" );
		return false;
	}

	if ( !g_pClientDLL->Init() )
	{
		printf( "Client DLL failed to initialize!\n" );
		return false;
	}

	g_pClientGameObjectFactory = g_pClientDLL->GetGameObjectFactory();

	if ( g_pClientGameObjectFactory == nullptr )
	{
		printf( "Failed to get GameObject factory from client DLL!\n" );
		return false;
	}

	if ( !GetFactory()->LoadModule( "bin/server" + string( DLL_EXTENSION ) ) )
		return false;

	g_pServerDLL = ( IServerDLL* )GetFactory()->GetInterface( SERVER_INTERFACE_VERSION );

	if ( g_pServerDLL == nullptr )
	{
		printf( "Failed to get server interface!\n" );
		return false;
	}

	if ( !g_pServerDLL->Init() )
	{
		printf( "Server DLL failed to initialize!\n" );
		return false;
	}

	g_pServerGameObjectFactory = g_pServerDLL->GetGameObjectFactory();

	if ( g_pServerGameObjectFactory == nullptr )
	{
		printf( "Failed to get GameObject factory from server DLL!\n" );
		return false;
	}

	if ( enet_initialize() != 0 )
	{
		printf( "An error occured while initializing ENet.\n" );
		return false;
	}

	m_bNetworkInitialized = true;

	LoadMap( "test.ambsp" );

	if ( !m_ServerHost.Init() )
		return false;

	if ( !m_Client.Init() )
		return false;

	return true;
}

void Engine::PostInit()
{
	g_pRenderer->PostInit();
}

void Engine::Shutdown()
{
	m_ServerHost.Shutdown();

	if ( g_pRenderer )
		g_pRenderer->Shutdown();

	if ( m_bNetworkInitialized )
		enet_deinitialize();

	if ( m_bSDLInitialized )
		SDL_Quit();
}

int Engine::RunMainLoop()
{
	using namespace std::literals::chrono_literals;
	m_bDone = false;

	m_flDeltaTime = 0.0f;
	int frameTicks = 0;

	std::chrono::high_resolution_clock::time_point oldTime;
	m_CurrentTime = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point nextFramePrint = m_CurrentTime + 1s;

	while ( !m_bDone )
	{
		oldTime = m_CurrentTime;
		m_CurrentTime = std::chrono::high_resolution_clock::now();
		auto delta = std::chrono::duration_cast< std::chrono::duration < float > >( m_CurrentTime - oldTime );
		m_flDeltaTime = delta.count();

		m_ServerHost.Update();
		m_Client.Update();

		if ( nextFramePrint <= m_CurrentTime )
		{
            printf( "FPS: %d\n", frameTicks );
            frameTicks = 0;
            nextFramePrint = m_CurrentTime + 1s;
		}

		g_pInput->Update();

		if ( g_pInput->IsButtonJustPressed( "Quit" ) )
			m_bDone = true;

		g_pRenderer->GetViewPort()->UpdateViewPort();

		g_pRenderer->Clear();
		g_pRenderer->DrawScene();
		g_pRenderer->Swap();

		//std::this_thread::sleep_for( 1ms );
		//std::this_thread::yield();
		frameTicks++;
	}

	return 0;
}

void Engine::LoadMap( const string &mapName )
{
	std::ifstream map;
	map.open( string( GAME_DIR ) + "maps/" + mapName );

	if ( map.is_open() )
	{
		uint64_t version = 0;
		uint32_t numObjects = 0;
		map.read( ( char* )&version, sizeof( version ) );
		map.read( ( char* )&numObjects, sizeof( numObjects ) );

		for ( uint32_t i = 0; i < numObjects; i++ )
		{
			uint64_t objectNameLen = 0;
			map.read( ( char* )&objectNameLen, sizeof( objectNameLen ) );

			string objectName( objectNameLen, ' ' );
			map.read( objectName.data(), objectNameLen );

			std::cout << "Creating object " << objectName << std::endl;
			IGameObject *pGameObject = g_pServerGameObjectFactory->CreateGameObject( objectName );
			GetEngineServer_Internal()->SetupGameObject( pGameObject );
		}

		map.close();
	}
	else
	{
		printf( "Failed to load map!\n" );
		return;
	}
}

static DLLInterface < IEngine, Engine > s_Engine( ENGINE_INTERFACE_VERSION );
Engine *GetEngine_Internal() { return s_Engine.GetInternal(); }
