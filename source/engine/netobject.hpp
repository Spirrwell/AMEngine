#ifndef NETOBJECT_HPP
#define NETOBJECT_HPP

#include "inetobject.hpp"
#include "igameobject.hpp"

struct NetData_t
{
	void *pData;
	uint32_t dataLength;
};

// This isn't really safe, but does the job!
typedef NetData_t InitWorldState_t;
typedef NetData_t SyncWorldState_t;

struct NetObject : public INetObject
{
	NetObject();

	const uint32_t &GetNetIndex() override { return m_iNetIndex; }
	void AttachToGameObject( IGameObject *pGameObject );

	IGameObject *GetOwner() { return m_pGameObject; }

	// Be careful with this
	uint32_t m_iNetIndex;

private:
	IGameObject *m_pGameObject;
};

#endif // NETOBJECT_HPP