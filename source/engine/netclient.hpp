#ifndef NETCLIENT_HPP
#define NETCLIENT_HPP

#include "enet/enet.h"

class NetworkClient
{
public:
	NetworkClient();
	~NetworkClient();

	bool Init();
	void Shutdown();

	void Update();

private:
	ENetHost *m_pClient;
	ENetPeer *m_pPeer;
	ENetAddress m_hAddress;
};

#endif // NETCLIENT_HPP