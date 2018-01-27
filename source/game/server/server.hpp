#ifndef SERVER_HPP
#define SERVER_HPP

#include "iserver.hpp"

class ServerDLL : public IServerDLL
{
	virtual bool Init() override;
	IGameObjectFactory *GetGameObjectFactory() override;
};

#endif // SERVER_HPP