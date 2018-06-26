#ifndef IINPUT_HPP
#define IINPUT_HPP

#include "string.hpp"

class IInput
{
public:
	virtual bool Init() = 0;

	virtual bool IsButtonPressed( const string &strButtonName ) = 0;
	virtual bool IsButtonReleased( const string &strButtonName ) = 0;
	virtual bool IsButtonJustPressed( const string &strButtonName ) = 0;
	virtual bool IsButtonJustReleased( const string &strButtonName ) = 0;

	virtual void Update() = 0;

	// Mouse delta is an integer, but internally it's easier to just represent it as a float
	virtual float GetMouseDeltaX() = 0;
	virtual float GetMouseDeltaY() = 0;
};

#define INPUT_INTERFACE_VERSION "INPUTV01"

#endif // IINPUT_HPP