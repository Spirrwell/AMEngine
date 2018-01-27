#include "materialsystem.hpp"
#include "factory/ifactory.hpp"
#include "interface.hpp"
#include "material.hpp"

IRenderer *g_pRenderer = nullptr;

MaterialSystem::MaterialSystem()
{

}

bool MaterialSystem::Init()
{
	g_pRenderer = ( IRenderer* )GetFactory()->GetInterface( RENDERER_INTERFACE_OPENGL );

	if ( g_pRenderer == nullptr )
	{
		printf( "MaterialSystem: Failed to get renderer interface!\n" );
		return false;
	}

	return true;
}

IMaterial *MaterialSystem::CreateMaterial( const std::string &materialName )
{
	return new Material( std::string( GAME_DIR ) + materialName );
}

static DLLInterface < IMaterialSystem, MaterialSystem > s_MaterialSystem( MATERIALSYSTEM_INTERFACE_VERSION );