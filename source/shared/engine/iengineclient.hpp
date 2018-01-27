#ifndef IENGINECLIENT_HPP
#define IENGINECLIENT_HPP

#include "inetworktable.hpp"

class IEngineClient
{
public:
	virtual ~IEngineClient() {}

	//virtual void RegisterNetworkTable( INetworkTable *pNetTable ) = 0;
};

#define IENGINECLIENT_INTERFACE_VERSION "ENGINECLIENTV001"

#endif