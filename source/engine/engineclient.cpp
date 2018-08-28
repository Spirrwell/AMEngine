#include "engineclient.hpp"
#include "interface.hpp"
#include "amlib/memoryreader.hpp"
#include "igameobjectfactory.hpp"

#include <iostream>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern IGameObjectFactory *g_pClientGameObjectFactory;

EngineClient::~EngineClient()
{
	Shutdown();
}

void EngineClient::Shutdown()
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

//void EngineClient::RegisterNetworkTable( INetworkTable *pNetTable )
//{
	//m_pNetworkTables.push_back( pNetTable );
//}

void EngineClient::UpdateNetworkTables()
{

}

void EngineClient::InitWorldState( InitWorldState_t worldState )
{
	MemoryReader memReader( ( char* )worldState.pData, ( size_t )worldState.dataLength );

	uint32_t netObjectCount = 0;
	uint32_t gameObjectCount = 0;

	memReader.read( ( char* )&netObjectCount, sizeof ( netObjectCount ) );
	memReader.read( ( char* )&gameObjectCount, sizeof( gameObjectCount ) );

	m_pNetObjects.resize( ( size_t )netObjectCount );

	for ( uint32_t i = 0; i < gameObjectCount; i++ )
	{
		uint64_t hashID = 0;
		memReader.read( ( char* )&hashID, sizeof( hashID ) );

		uint32_t netIndex = 0;
		memReader.read( ( char* )&netIndex, sizeof( netIndex ) );

		IGameObject *pGameObject = g_pClientGameObjectFactory->CreateGameObject( hashID );

		std::cout << "Creating client game object: " << pGameObject->GetRegister()->GetName() << std::endl;

		NetObject *pNetObject = new NetObject;
		pNetObject->m_iNetIndex = netIndex;

		m_pNetObjects[ netIndex ] = pNetObject;
		pNetObject->AttachToGameObject( pGameObject );

		pGameObject->SetupNetworkTable();

		char *pNetworkVarData = ( &memReader.data()[ memReader.pos() ] );
		size_t length = memReader.size() - memReader.pos();

		pGameObject->GetNetworkTable()->UpdateNetworkTable( pNetworkVarData, length );

		pGameObject->Think();

		m_pGameObjects.push_back( pGameObject );
	}
}

void EngineClient::SyncWorldState( SyncWorldState_t syncWorldState )
{
	MemoryReader memReader( ( char* )syncWorldState.pData, ( size_t )syncWorldState.dataLength );

	uint32_t netObjectCount = 0;
	memReader.read( ( char* )&netObjectCount, sizeof( netObjectCount ) );

	for ( uint32_t i = 0; i < netObjectCount; i++ )
	{
		uint32_t netIndex = 0;
		memReader.read( ( char* )&netIndex, sizeof( netIndex ) );

		NetObject *pNetObject = m_pNetObjects[ netIndex ];
		IGameObject *pGameObject = pNetObject->GetOwner();

		size_t length = 0;
		std::vector< INetworkVar * > pNetVars = pGameObject->GetNetworkTable()->GetNetworkVars();
		for ( INetworkVar *pNetVar : pNetVars )
			length += pNetVar->sizeofElement();

		char *pNetworkVarData = ( char* )malloc( length );
		std::memcpy( pNetworkVarData, ( &memReader.data()[ memReader.pos() ] ), length );

		pGameObject->GetNetworkTable()->UpdateNetworkTable( pNetworkVarData, length );
		free( pNetworkVarData );
	}
}

static DLLInterface < IEngineClient, EngineClient > s_EngineClient( IENGINECLIENT_INTERFACE_VERSION );
EngineClient *GetEngineClient_Internal() { return s_EngineClient.GetInternal(); }