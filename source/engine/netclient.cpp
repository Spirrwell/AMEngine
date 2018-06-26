#include "netclient.hpp"
#include "netmessage.hpp"
#include "netobject.hpp"
#include "engineclient.hpp"

#include <cstdio>

NetworkClient::NetworkClient()
{
	m_pClient = nullptr;
	m_pPeer = nullptr;
}

NetworkClient::~NetworkClient()
{
	Shutdown();
}

bool NetworkClient::Init()
{
	m_pClient = enet_host_create( nullptr, 1, NetMsg_MAX, 0, 0 );

	if ( m_pClient == nullptr )
	{
		printf( "An error occurred while trying to create an ENet client host.\n" );
		return false;
	}

	// For testing, we'll be connecting locally for now
	enet_address_set_host( &m_hAddress, "127.0.0.1" );
	//enet_address_set_host( &m_hAddress, "192.168.0.5" );
	m_hAddress.port = 1234;

	m_pPeer = enet_host_connect( m_pClient, &m_hAddress, NetMsg_MAX, 0 );

	if ( m_pPeer = nullptr )
	{
		printf( "No available peers for initiating an ENet connection.\n" );
		return false;
	}

	return true;
}

void NetworkClient::Shutdown()
{
	if ( m_pClient != nullptr )
	{
		enet_host_destroy( m_pClient );
		m_pClient = nullptr;
	}

	// ENet cleans these up automatically?
	m_pPeer = nullptr;
}

void NetworkClient::Update()
{
	ENetEvent netEvent;

	while ( enet_host_service( m_pClient, &netEvent, 0 ) > 0 )
	{
		switch ( netEvent.type )
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{
				/*printf
				(
					"A packet of length %u was received on channel %u.\n",
					( unsigned int )netEvent.packet->dataLength,
					netEvent.channelID
				);*/

				switch ( netEvent.channelID )
				{
					case NetMsg_InitWorld:
					{
						printf( "Initializing world.\n" );

						InitWorldState_t worldState = { netEvent.packet->data, ( uint32_t )netEvent.packet->dataLength };
						GetEngineClient_Internal()->InitWorldState( worldState );

						break;
					}
				}

				enet_packet_destroy( netEvent.packet );

				break;
			}
		}
	}
}