#ifndef IGAMEOBJECTFACTORY_HPP
#define IGAMEOBJECTFACTORY_HPP

#include <string>

#include "igameobject.hpp"

class IGameObjectFactory
{
public:
	virtual ~IGameObjectFactory() {}

	virtual void RegisterGameObjectCreateFunc( const std::string &gameObjectName, void *( *pCreateFn )() ) = 0;
	virtual IGameObject *CreateGameObject( const std::string &gameObjectName ) = 0;
	virtual IGameObject *CreateGameObject( uint64_t hashID ) = 0;

	virtual uint64_t GetHashID( const std::string &gameObjectName ) = 0;
};

class IGameObjectRegister
{
public:
	virtual const std::string &GetName() = 0;
};

#endif // IGAMEOBJECTFACTORY_HPP