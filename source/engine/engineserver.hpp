#ifndef ENGINESERVER_HPP
#define ENGINESERVER_HPP

#include "iengineserver.hpp"
#include "igameobject.hpp"
#include "netobject.hpp"

#include <vector>

class EngineServer : public IEngineServer
{
public:
	virtual ~EngineServer();

	void Shutdown();

	//void RegisterNetworkTable( INetworkTable *pNetTable );
	void UpdateNetworkTables();
	void SetupGameObject( IGameObject *pGameObject );

	std::vector < IGameObject * > &GetGameObjects() { return m_pGameObjects; }

	InitWorldState_t GetInitWorldState();
	SyncWorldState_t GetSyncWorldState();

private:
	std::vector < IGameObject * > m_pGameObjects;
	std::vector < NetObject * > m_pNetObjects;
	//std::vector< INetworkTable * > m_pNetworkTables;
};

extern EngineServer *GetEngineServer_Internal();

#endif // ENGINESERVER_HPP