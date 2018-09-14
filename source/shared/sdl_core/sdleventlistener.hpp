#ifndef SDLEVENTLISTENER_HPP
#define SDLEVENTLISTENER_HPP

#include "SDL.h"
#include "sdl_core/isdl_core.hpp"

class SDLEventListener
{
public:
	SDLEventListener()
	{
		GetSDL_Core()->AddSDLEventListener( this );
	}

	virtual ~SDLEventListener()
	{
		GetSDL_Core()->RemoveSDLEventListener( this );
	}

	virtual void ProcessEvent( const SDL_Event &event ) {};
};

#endif // SDLEVENTLISTENER_HPP