#include "netobject.hpp"

NetObject::NetObject()
{
	m_pGameObject = nullptr;
}

void NetObject::AttachToGameObject( IGameObject *pGameObject )
{
	pGameObject->SetNetObject( this );
	m_pGameObject = pGameObject;
}