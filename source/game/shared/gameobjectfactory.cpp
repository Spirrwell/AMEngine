#include "gameobjectfactory.hpp"

#include <functional>

void GameObjectFactory::RegisterGameObjectCreateFunc( const std::string &gameObjectName, void *( *pCreateFn )() )
{
	m_mapGameObjectCreateFunctions[ gameObjectName ] = pCreateFn;
	uint64_t hash = std::hash < std::string >{}( gameObjectName );

	m_mapGameObjectNameHash[ hash ] = gameObjectName;
}

IGameObject *GameObjectFactory::CreateGameObject( const std::string &gameObjectName )
{
	return static_cast< IGameObject* >( m_mapGameObjectCreateFunctions[ gameObjectName ]() );
}

IGameObject *GameObjectFactory::CreateGameObject( uint64_t hashID )
{
	std::string gameObjectName = m_mapGameObjectNameHash[ hashID ];
	return CreateGameObject( gameObjectName );
}

uint64_t GameObjectFactory::GetHashID( const std::string &gameObjectName )
{
	uint64_t hashID = 0;
	for ( auto &kV : m_mapGameObjectNameHash )
	{
		if ( kV.second == gameObjectName )
		{
			hashID = kV.first;
			break;
		}
	}

	return hashID;
}

IGameObjectFactory *GetGameObjectFactory()
{
	static GameObjectFactory s_GameObjectFactory;
	return &s_GameObjectFactory;
}