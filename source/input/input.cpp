#include <cstring>

#include "input.hpp"
#include "interface.hpp"
#include "engine/iengine.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IEngine *g_pEngine = nullptr;

Input::Input()
{
	m_pCurrentKeyboardState = nullptr;
	m_pOldKeyboardState = nullptr;
}

Input::~Input()
{
	if ( m_pOldKeyboardState )
		delete[] m_pOldKeyboardState;
}

bool Input::Init()
{
	g_pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );

	if ( g_pEngine == nullptr )
	{
		printf( "Input: Failed to get engine interface!\n" );
		return false;
	}

	m_pCurrentKeyboardState = SDL_GetKeyboardState( &m_iNumKeys );
	m_pOldKeyboardState = new Uint8[ m_iNumKeys ];

	// TODO: Create config file with dynamic button mapping
	m_mapButtons[ "Forward" ] = SDL_SCANCODE_W;
	m_mapButtons[ "Backward" ] = SDL_SCANCODE_S;
	m_mapButtons[ "Left" ] = SDL_SCANCODE_A;
	m_mapButtons[ "Right" ] = SDL_SCANCODE_D;
	m_mapButtons[ "Jump" ] = SDL_SCANCODE_SPACE;
	m_mapButtons[ "Quit" ] = SDL_SCANCODE_ESCAPE;
	m_mapButtons[ "TurnLeft" ] = SDL_SCANCODE_LEFT;
	m_mapButtons[ "TurnRight" ] = SDL_SCANCODE_RIGHT;
	m_mapButtons[ "Crouch" ] = SDL_SCANCODE_LCTRL;

	return true;
}

// May want to do checks to see if a button name is valid
bool Input::IsButtonPressed( const string &strButtonName )
{
	return ( m_pCurrentKeyboardState[ m_mapButtons[ strButtonName ] ] == 1 );
}

bool Input::IsButtonReleased( const string &strButtonName )
{
	return !IsButtonPressed( strButtonName );
}

bool Input::IsButtonJustPressed( const string &strButtonName )
{
	return ( m_pCurrentKeyboardState[ m_mapButtons[ strButtonName ] ] == 1 && m_pOldKeyboardState[ m_mapButtons[ strButtonName ] ] == 0 );
}

bool Input::IsButtonJustReleased( const string &strButtonName )
{
	return ( m_pCurrentKeyboardState[ m_mapButtons[ strButtonName ] ] == 0 && m_pOldKeyboardState[ m_mapButtons[ strButtonName ] ] == 1 );
}

void Input::Update()
{
	m_flMouseDeltaX = 0.0f;
	m_flMouseDeltaY = 0.0f;

	// Note: We allocate the old keyboard state in the Init function
	// Also no need to call SDL_GetKeyboardState as the state ponter is updated when events are processed, hence SDL_PumpEvents()
	std::memcpy( m_pOldKeyboardState, m_pCurrentKeyboardState, sizeof( Uint8 ) * m_iNumKeys );
	SDL_PumpEvents();

	//SDL_GetRelativeMouseState( &m_iCurMouseX, &m_iCurMouseY );
	//m_flMouseDeltaX = ( float )m_iCurMouseX;
	//m_flMouseDeltaY = ( float )m_iCurMouseY;

	while ( SDL_PollEvent( &m_event ) )
	{
		switch ( m_event.type )
		{
		case SDL_QUIT:
			g_pEngine->SignalTerminate();
			break;
		case SDL_MOUSEMOTION:
			m_flMouseDeltaX = ( float ) m_event.motion.xrel;
			m_flMouseDeltaY = ( float ) m_event.motion.yrel;
			break;
		}
	}

	//printf( "X: %f Y: %f\n", m_flMouseDeltaX, m_flMouseDeltaY );
	//printf( "X: %d Y: %d\n", m_iCurMouseX, m_iCurMouseY );
}

static DLLInterface < IInput, Input > s_Input( INPUT_INTERFACE_VERSION );