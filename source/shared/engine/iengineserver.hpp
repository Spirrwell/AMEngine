#ifndef IENGINESERVER_HPP
#define IENGINESERVER_HPP

#include "inetworktable.hpp"

class IEngineServer
{
public:
	virtual ~IEngineServer() = default;

	//virtual void RegisterNetworkTable( INetworkTable *pNetTable ) = 0;
};

#define IENGINESERVER_INTERFACE_VERSION "ENGINESERVERV001"

#endif // IENGINESERVER_HPP