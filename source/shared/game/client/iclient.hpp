#ifndef ICLIENT_HPP
#define ICLIENT_HPP

#include "engine/igameobjectfactory.hpp"

class IClientDLL
{
public:
	virtual bool Init() = 0;

	virtual IGameObjectFactory *GetGameObjectFactory() = 0;
};

#define CLIENT_INTERFACE_VERSION "CLIENTV01"

#endif // ICLIENT_ HPP