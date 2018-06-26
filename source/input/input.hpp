#ifndef INPUT_HPP
#define INPUT_HPP

#include <map>
#include <stdint.h>

#include "iinput.hpp"
#include "SDL.h"

class Input : public IInput
{
public:
	Input();
	virtual ~Input();

	virtual bool Init();

	virtual bool IsButtonPressed( const string &strButtonName );
	virtual bool IsButtonReleased( const string &strButtonName );
	virtual bool IsButtonJustPressed( const string &strButtonName );
	virtual bool IsButtonJustReleased( const string &strButtonName );

	virtual void Update();

	virtual float GetMouseDeltaX() { return m_flMouseDeltaX; }
	virtual float GetMouseDeltaY() { return m_flMouseDeltaY; }

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