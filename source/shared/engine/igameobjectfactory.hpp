#ifndef IGAMEOBJECTFACTORY_HPP
#define IGAMEOBJECTFACTORY_HPP

#include "string.hpp"

#include "igameobject.hpp"

class IGameObjectFactory
{
public:
	virtual ~IGameObjectFactory() = default;

	virtual void RegisterGameObjectCreateFunc( const string &gameObjectName, void *( *pCreateFn )() ) = 0;
	virtual IGameObject *CreateGameObject( const string &gameObjectName ) = 0;
	virtual IGameObject *CreateGameObject( uint64_t hashID ) = 0;

	virtual uint64_t GetHashID( const string &gameObjectName ) = 0;
};

class IGameObjectRegister
{
public:
	virtual const string &GetName() = 0;
};

#endif // IGAMEOBJECTFACTORY_HPP