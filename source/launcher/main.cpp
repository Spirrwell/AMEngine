#include <iostream>
#include <stdlib.h>

#include "platform.hpp"
#include "engine/iengine.hpp"
#include "engine/ienginelauncher.hpp"

#include "factory/ifactory.hpp"
#include "memory_system/imemory_system.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IEngineLauncher *g_pEngineLauncher = nullptr;
IEngine *g_pEngine = nullptr;

bool LoadDependencies()
{
	if ( !GetFactory()->LoadModule( "bin/engine" + string( DLL_EXTENSION ) ) )
	{
		printf( "Failed to load engine library.\n" );
		return false;
	}

	g_pEngineLauncher = ( IEngineLauncher* )GetFactory()->GetInterface( ENGINELAUNCHER_INTERFACE_VERSION );

	if ( g_pEngineLauncher == nullptr )
	{
		printf( "Failed to get engine launcher interface!\n" );
		return false;
	}

	if ( !g_pEngineLauncher->Init() )
		return false;

	g_pEngineLauncher->PostInit();

	g_pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );

	if ( !g_pEngine )
	{
		printf( "Failed to get engine interface!\n" );
		return false;
	}

	return true;
}

void Shutdown()
{
	if ( g_pEngineLauncher )
		g_pEngineLauncher->Shutdown();

	GetFactory()->Shutdown();

	g_pEngine = nullptr;
	g_pEngineLauncher = nullptr;
}

int main( int argc, char *argv[] )
{
	bool bShouldRun = true;
	while ( bShouldRun )
	{
		if ( !LoadDependencies() )
		{
			Shutdown();
			std::cin.get();
			return -1;
		}

		g_pEngine->RunMainLoop();
		Shutdown();

		/*string input;
		std::cout << "Do you want to restart the engine? (Y or N)\n";
		std::cin >> input;

		if ( input == "N" || input == "n" )*/
			bShouldRun = false;
	}

	GetMemTracker()->PrintAllocations();
	std::cin.get();

	return 0;
}
