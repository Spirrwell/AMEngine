#include "nethost.hpp"
#include "netmessage.hpp"
#include "engineserver.hpp"
#include "engine.hpp"

#include <cstdio>
#include <string>
#include <iostream>

extern Engine *GetEngine_Internal();

std::string IPToString( enet_uint32 IP )
{
	return
		std::to_string( IP & 0x000000FF ) +
		"." +
		std::to_string( ( IP & 0x0000FF00 ) >> 8 ) +
		"." +
		std::to_string( ( IP & 0x00FF0000 ) >> 16 ) +
		"." +
		std::to_string( ( IP & 0xFF000000 ) >> 24 );
}

NetworkHost::NetworkHost()
{
	using namespace std::chrono_literals;
	m_pServer = nullptr;
}

NetworkHost::~NetworkHost()
{
	Shutdown();
}

bool NetworkHost::Init()
{
	m_hAddress.host = ENET_HOST_ANY;
	m_hAddress.port = 1234;

	m_pServer = enet_host_create( &m_hAddress, 32, NetMsg_MAX, 0, 0 );

	if ( m_pServer == nullptr )
	{
		printf( "An error occurred while trying to create an ENet server host.\n" );
		return false;
	}

	return true;
}

void NetworkHost::Shutdown()
{
	if ( m_pServer != nullptr )
	{
		enet_host_destroy( m_pServer );
		m_pServer = nullptr;
	}
}

void NetworkHost::Update()
{
	using namespace std::chrono_literals;

	if ( m_pPeers.size() > 0 && m_UpdateTime <= GetEngine_Internal()->GetCurTime() )
	{
		SyncWorldState_t syncWorldState = GetEngineServer_Internal()->GetSyncWorldState();
		ENetPacket *pPacket = enet_packet_create( syncWorldState.pData, syncWorldState.dataLength, ENET_PACKET_FLAG_RELIABLE );
		enet_host_broadcast( m_pServer, NetMsg_SyncWorld, pPacket );

		free( syncWorldState.pData );
	}

	if ( GetEngine_Internal()->GetCurTime() > m_UpdateTime )
		m_UpdateTime = GetEngine_Internal()->GetCurTime() + 50ms;

	ENetEvent netEvent;

	while ( enet_host_service( m_pServer, &netEvent, 0 ) > 0 )
	{
		switch ( netEvent.type )
		{
			case ENET_EVENT_TYPE_CONNECT:
			{
				std::cout << "A new client connected from " << IPToString( netEvent.peer->address.host ) << ":" << netEvent.peer->address.port << std::endl;
				netEvent.peer->data = ( void* )"Client information";

				m_pPeers.push_back( netEvent.peer );

				InitWorldState_t worldState = GetEngineServer_Internal()->GetInitWorldState();

				if ( worldState.pData != nullptr )
				{
					ENetPacket *pPacket = enet_packet_create( worldState.pData, ( size_t )worldState.dataLength, ENET_PACKET_FLAG_RELIABLE );
					enet_peer_send( netEvent.peer, NetMsg_InitWorld, pPacket );

					free( worldState.pData );
				}

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				/*printf
				(
					"A packet of length %u was received from %s on channel %u.\n",
					( unsigned int )netEvent.packet->dataLength,
					( char* )netEvent.peer->data,
					netEvent.channelID
				);*/

				enet_packet_destroy( netEvent.packet );

				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				for ( unsigned int i = 0; i < ( unsigned int )m_pPeers.size(); i++ )
				{
					if ( m_pPeers[i] == netEvent.peer )
					{
						m_pPeers.erase( m_pPeers.begin() + i );
						break;
					}
				}
				break;
			}
		}
	}
}