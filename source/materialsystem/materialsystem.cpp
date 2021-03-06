#include "materialsystem.hpp"
#include "factory/ifactory.hpp"
#include "interface.hpp"
#include "material.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IRenderer *g_pRenderer = nullptr;

MaterialSystem::MaterialSystem()
{

}

bool MaterialSystem::Init()
{
	g_pRenderer = ( IRenderer* )GetFactory()->GetInterface( RENDERER_INTERFACE );

	if ( g_pRenderer == nullptr )
	{
		printf( "MaterialSystem: Failed to get renderer interface!\n" );
		return false;
	}

	return true;
}

IMaterial *MaterialSystem::CreateMaterial( const string &materialName )
{
	return new Material( string( GAME_DIR ) + materialName );
}

static DLLInterface < IMaterialSystem, MaterialSystem > s_MaterialSystem( MATERIALSYSTEM_INTERFACE_VERSION );