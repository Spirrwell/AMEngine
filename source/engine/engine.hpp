#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>
#include <chrono>

#include "iengine.hpp"
#include "interface.hpp"
#include "igameobject.hpp"
#include "netobject.hpp"
#include "nethost.hpp"
#include "netclient.hpp"

#include "SDL.h"

class Engine : public IEngine
{
public:
	Engine();
	virtual ~Engine();

	virtual bool Init();
	virtual void PostInit();

	virtual void Shutdown();

	virtual int RunMainLoop();

	virtual const float &GetAspectRatio() { return m_flAspectRatio; }

	virtual float GetDeltaTime() { return m_flDeltaTime; };

	virtual void SignalTerminate() { m_bDone = true; }

	virtual config &GetConfig() { return m_Config; }

	void LoadMap( const string &mapname );

	std::chrono::high_resolution_clock::time_point GetCurTime() { return m_CurrentTime; }

private:
	std::chrono::high_resolution_clock::time_point m_CurrentTime;

	NetworkHost m_ServerHost;
	NetworkClient m_Client;

	bool m_bSDLInitialized;

	config m_Config;

	float m_flAspectRatio;
	float m_flDeltaTime;

	bool m_bDone;
	bool m_bNetworkInitialized;
};

#endif // ENGINE_HPP