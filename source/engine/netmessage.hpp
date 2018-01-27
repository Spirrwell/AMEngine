#ifndef NETMESSAGE_HPP
#define NETMESSAGE_HPP

#include "enet/enet.h"

// Messages that relate to the channel in ENet they use
enum NetMessageType
{
	NetMsg_InitWorld,		// Initial world state sent to the client when they connect
	NetMsg_SyncWorld,		// Sync everything in the world when client connects
	NetMsg_NetworkTable,	// Periodic updates to all clients
	NetMsg_MAX
};

class INetMessage
{
public:
	virtual ~INetMessage() {}

	virtual const NetMessageType GetType() const = 0;
};

class NetMessage_NetTable : public INetMessage
{
public:
	NetMessage_NetTable();
	const NetMessageType GetType() const override { return NetMsg_NetworkTable; }
};

#endif // NETMESSAGE_HPP