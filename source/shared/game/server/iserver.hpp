#ifndef ISERVER_HPP
#define ISERVER_HPP

#include "engine/igameobjectfactory.hpp"

class IServerDLL
{
public:
	virtual bool Init() = 0;
	virtual IGameObjectFactory *GetGameObjectFactory() = 0;
};

#define SERVER_INTERFACE_VERSION "SERVERV01"

#endif // ISERVER_HPP