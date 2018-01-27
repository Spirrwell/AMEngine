#ifndef IENGINE_HPP
#define IENGINE_HPP

#include "platform.hpp"
#include "iinterface.hpp"
#include "config.hpp"

class IEngine
{
public:
    virtual ~IEngine() {}

	virtual bool Init() = 0;
	virtual void PostInit() = 0;

	virtual int RunMainLoop() = 0;

	virtual const float &GetAspectRatio() = 0;

	virtual float GetDeltaTime() = 0;

	virtual void SignalTerminate() = 0;

	virtual config &GetConfig() = 0;
};

#define ENGINE_INTERFACE_VERSION "AMENGINEV01"

#endif // IENGINE_HPP