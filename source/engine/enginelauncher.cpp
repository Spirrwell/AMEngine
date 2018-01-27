#include "enginelauncher.hpp"
#include "engine.hpp"

extern Engine *GetEngine_Internal();

EngineLauncher::~EngineLauncher()
{
}

bool EngineLauncher::Init()
{
	return GetEngine_Internal()->Init();
}

void EngineLauncher::PostInit()
{
	GetEngine_Internal()->PostInit();
}

void EngineLauncher::Shutdown()
{
	GetEngine_Internal()->Shutdown();
}

DLLInterface < IEngineLauncher, EngineLauncher > s_EngineLauncher( ENGINELAUNCHER_INTERFACE_VERSION );
