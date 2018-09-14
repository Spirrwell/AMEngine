#ifndef SDL_CORE_HPP
#define SDL_CORE_HPP

#include "isdl_core.hpp"
#include "SDL.h"

#include "sdleventlistener.hpp"

#include <vector>

class SDL_Core : public ISDL_Core
{
public:
	void ProcessEvents() override;

	void AddSDLEventListener( SDLEventListener *pListener ) override;
	void RemoveSDLEventListener( SDLEventListener *pListener ) override;

private:
	std::vector< SDLEventListener * > m_pSDLEventListeners;
	SDL_Event m_Event;
};

#endif // SDL_CORE_HPP