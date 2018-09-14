#include "sdl_core.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

void SDL_Core::ProcessEvents()
{
	SDL_PumpEvents();

	while ( SDL_PollEvent( &m_Event ) )
	{
		for ( auto &sdlEventListener : m_pSDLEventListeners )
		{
			sdlEventListener->ProcessEvent( m_Event );
		}
	}
}

void SDL_Core::AddSDLEventListener( SDLEventListener *pListener )
{
	m_pSDLEventListeners.push_back( pListener );
}

void SDL_Core::RemoveSDLEventListener( SDLEventListener *pListener )
{
	for ( auto it = m_pSDLEventListeners.begin(); it != m_pSDLEventListeners.end(); ++it )
	{
		if ( *it == pListener )
		{
			m_pSDLEventListeners.erase( it );
			return;
		}
	}
}

static SDL_Core s_SDL_Core;
ISDL_Core *GetSDL_Core() { return static_cast< ISDL_Core* >( &s_SDL_Core ); }