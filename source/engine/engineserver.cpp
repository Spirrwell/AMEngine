#include "engineserver.hpp"
#include "interface.hpp"
#include "amlib/memorywriter.hpp"
#include "igameobjectfactory.hpp"

#include <cassert>
#include <iostream>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern IGameObjectFactory *g_pServerGameObjectFactory;

EngineServer::~EngineServer()
{
	Shutdown();
}

void EngineServer::Shutdown()
{
	for ( IGameObject *pGameObject : m_pGameObjects )
	{
		pGameObject->OnRemove();
		delete pGameObject;
	}

	m_pGameObjects.clear();

	for ( NetObject *pNetObject : m_pNetObjects )
		delete pNetObject;

	m_pNetObjects.clear();
}

//void EngineServer::RegisterNetworkTable( INetworkTable *pNetTable )
//{
	//m_pNetworkTables.push_back( pNetTable );
//}

void EngineServer::UpdateNetworkTables()
{
}

void EngineServer::SetupGameObject( IGameObject *pGameObject )
{
	assert( pGameObject != nullptr );

	NetObject *pNetObject = new NetObject;
	pNetObject->m_iNetIndex = ( int )std::distance( m_pNetObjects.begin(), m_pNetObjects.insert( m_pNetObjects.end(), pNetObject ) );

	pNetObject->AttachToGameObject( pGameObject );
	pGameObject->SetupNetworkTable();

	m_pGameObjects.push_back( pGameObject );
}

InitWorldState_t EngineServer::GetInitWorldState()
{
	InitWorldState_t worldState = { nullptr, 0 };

	MemoryWriter memWriter;

	uint32_t netObjectCount = ( uint32_t )m_pNetObjects.size();
	uint32_t gameObjectCount = ( uint32_t )m_pGameObjects.size();

	memWriter.write( ( char* )&netObjectCount, sizeof ( netObjectCount ) );
	memWriter.write( ( char* )&gameObjectCount, sizeof( gameObjectCount ) );
	
	for ( IGameObject *pGameObject : m_pGameObjects )
	{
		string gameObjectName = pGameObject->GetRegister()->GetName();
		stprintf( "Registered Name: " ) << gameObjectName << std::endl;
		uint64_t hashID = g_pServerGameObjectFactory->GetHashID( gameObjectName );

		if ( hashID == 0 )
		{
			stprintf( "An error occurred looking up a game object!\n" );
			return worldState;
		}

		NetObject *pNetObject = static_cast< NetObject* >( pGameObject->GetNetObject() );

		if ( pNetObject == nullptr )
		{
			stprintf( "An error occurred while looking up net object!\n" );
			return worldState;
		}

		uint32_t netIndex = pNetObject->m_iNetIndex;

		memWriter.write( ( char* )&hashID, sizeof ( hashID ) );
		memWriter.write( ( char* )&netIndex, sizeof( netIndex ) );

		std::vector < INetworkVar * > pNetVars = pGameObject->GetNetworkTable()->GetNetworkVars();

		for ( INetworkVar *pNetVar : pNetVars )
			memWriter.write( ( char* )pNetVar->data(), pNetVar->sizeofElement() );
	}
	
	// Keep in mind MemoryWriter mallocs this data, use free() on it later, not delete
	worldState.pData = ( void* )memWriter.data();
	worldState.dataLength = ( uint32_t )memWriter.size();

	// Close writer, but don't free, worldState owns the data now
	memWriter.close();

	return worldState;
}

SyncWorldState_t EngineServer::GetSyncWorldState()
{
	SyncWorldState_t syncWorldState = { nullptr, 0 };

	MemoryWriter memWriter;

	uint32_t netObjectCount = ( uint32_t )m_pNetObjects.size();
	memWriter.write( ( char* )&netObjectCount, sizeof( netObjectCount ) );

	for ( IGameObject *pGameObject : m_pGameObjects )
	{
		INetObject *pNetObject = pGameObject->GetNetObject();
		uint32_t netIndex = pNetObject->GetNetIndex();

		memWriter.write( ( char* )&netIndex, sizeof( netIndex ) );
		std::vector < INetworkVar * > pNetVars = pGameObject->GetNetworkTable()->GetNetworkVars();

		for ( INetworkVar *pNetVar : pNetVars )
			memWriter.write( ( char* )pNetVar->data(), pNetVar->sizeofElement() );
	}

	// Keep in mind MemoryWriter mallocs this data, use free() on it later, not delete
	syncWorldState.pData = ( void* )memWriter.data();
	syncWorldState.dataLength = ( uint32_t )memWriter.size();

	// Close writer, but don't free, worldState owns the data now
	memWriter.close();

	return syncWorldState;
}

static DLLInterface < IEngineServer, EngineServer > s_EngineServer( IENGINESERVER_INTERFACE_VERSION );
EngineServer *GetEngineServer_Internal() { return s_EngineServer.GetInternal(); }