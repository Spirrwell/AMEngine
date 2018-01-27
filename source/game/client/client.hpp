#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "iclient.hpp"

class ClientDLL : public IClientDLL
{
public:
	virtual bool Init() override;

	IGameObjectFactory *GetGameObjectFactory() override;
};

#endif // CLIENT_HPP