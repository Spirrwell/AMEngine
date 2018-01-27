#ifndef ENGINECLIENT_HPP
#define ENGINECLIENT_HPP

#include "iengineclient.hpp"
#include "igameobject.hpp"
#include "netobject.hpp"

#include <vector>

class EngineClient : public IEngineClient
{
public:
	virtual ~EngineClient();
	void Shutdown();

	//void RegisterNetworkTable( INetworkTable *pNetTable ) override;
	void UpdateNetworkTables();

	void InitWorldState( InitWorldState_t worldState );
	void SyncWorldState( SyncWorldState_t syncWorldState );

private:
	//std::vector< INetworkTable * > m_pNetworkTables;
	std::vector < NetObject * > m_pNetObjects;
	std::vector < IGameObject * > m_pGameObjects;
};

extern EngineClient *GetEngineClient_Internal();

#endif // ENGINECLIENT_HPP