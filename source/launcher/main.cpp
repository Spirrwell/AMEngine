#include <iostream>
#include <stdlib.h>

#include "platform.hpp"
#include "engine/iengine.hpp"
#include "engine/ienginelauncher.hpp"

#include "factory/ifactory.hpp"

int main( int argc, char *argv[] )
{
	if ( !GetFactory()->LoadModule( "bin/engine" + std::string( DLL_EXTENSION ) ) )
	{
		printf( "Failed to load engine library.\n" );
		std::cin.get();
		return -1;
	}

	IEngineLauncher *pEngineLauncher = ( IEngineLauncher* )GetFactory()->GetInterface( ENGINELAUNCHER_INTERFACE_VERSION );

	if ( pEngineLauncher == nullptr )
	{
		printf( "Failed to get engine launcher interface!\n" );
		return -1;
	}

	if ( !pEngineLauncher->Init() )
		return -1;

	pEngineLauncher->PostInit();

	IEngine *pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );
	if ( pEngine )
	{
		pEngine->RunMainLoop();
	}

	pEngineLauncher->Shutdown();

	return 0;
}
