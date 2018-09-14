#ifndef INPUT_HPP
#define INPUT_HPP

#include <map>
#include <stdint.h>

#include "iinput.hpp"
#include "SDL.h"

#include "sdl_core/sdleventlistener.hpp"

class Input : public IInput, public SDLEventListener
{
public:
	Input();
	virtual ~Input();

	bool Init() override;

	bool IsButtonPressed( const string &strButtonName ) override;
	bool IsButtonReleased( const string &strButtonName ) override;
	bool IsButtonJustPressed( const string &strButtonName ) override;
	bool IsButtonJustReleased( const string &strButtonName ) override;

	void Update() override;
	void ProcessEvent( const SDL_Event &event ) override;

	float GetMouseDeltaX() override { return m_flMouseDeltaX; }
	float GetMouseDeltaY() override { return m_flMouseDeltaY; }

private:
	std::map< string, Uint8 > m_mapButtons;
	const Uint8 *m_pCurrentKeyboardState;
	Uint8 *m_pOldKeyboardState;

	int m_iNumKeys;

	SDL_Event m_event;

	float m_flMouseDeltaX;
	float m_flMouseDeltaY;
};

#endif // INPUT_HPP