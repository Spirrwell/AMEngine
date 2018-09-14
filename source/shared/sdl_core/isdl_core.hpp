#ifndef ISDL_CORE_HPP
#define ISDL_CORE_HPP

#include "platform.hpp"

class SDLEventListener;

class ISDL_Core
{
public:
	virtual void ProcessEvents() = 0;

	virtual void AddSDLEventListener( SDLEventListener *pListener ) = 0;
	virtual void RemoveSDLEventListener( SDLEventListener *pListener ) = 0;

};

SDL_CORE_INTERFACE ISDL_Core *GetSDL_Core();

#endif // ISDL_CORE_HPP