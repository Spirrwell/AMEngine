#ifndef NETHOST_HPP
#define NETHOST_HPP

#include "enet/enet.h"

#include <vector>
#include <chrono>

class NetworkHost
{
public:
	NetworkHost();
	~NetworkHost();

	bool Init();
	void Shutdown();

	void Update();

private:
	ENetAddress m_hAddress;
	ENetHost *m_pServer;

	std::vector< ENetPeer * > m_pPeers;

	std::chrono::high_resolution_clock::time_point m_UpdateTime;
};

#endif // NETHOST_HPP