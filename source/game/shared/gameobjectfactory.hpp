#ifndef GAMEOBJECTFACTORY_HPP
#define GAMEOBJECTFACTORY_HPP

#include "engine/igameobjectfactory.hpp"

#include <map>

#include "memlib/memoryoverride.hpp"

class GameObjectFactory : public IGameObjectFactory
{
public:
	void RegisterGameObjectCreateFunc( const string &gameObjectName, void *( *pCreateFn )() ) override;
	IGameObject *CreateGameObject( const string &gameObjectName ) override;
	IGameObject *CreateGameObject( uint64_t hashID ) override;
	uint64_t GetHashID( const string &gameObjectName ) override;

private:
	std::map < string, void *(*)() > m_mapGameObjectCreateFunctions;
	std::map < uint64_t, string > m_mapGameObjectNameHash;
};

IGameObjectFactory *GetGameObjectFactory();

template< class T >
class GameObjectRegister : public IGameObjectRegister
{
public:
	GameObjectRegister( const string &gameObjectName )
	{
		m_strName = gameObjectName;
		GetGameObjectFactory()->RegisterGameObjectCreateFunc( gameObjectName, &Create );
	}

	static void *Create()
	{
		return new T;
	}

	const string &GetName() override { return m_strName; }

private:
	string m_strName;
};

#include "memlib/memoryoverride_off.hpp"

#endif // GAMEOBJECTFACTORY_HPP