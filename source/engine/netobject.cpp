#include "netobject.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

NetObject::NetObject()
{
	m_pGameObject = nullptr;
}

void NetObject::AttachToGameObject( IGameObject *pGameObject )
{
	pGameObject->SetNetObject( this );
	m_pGameObject = pGameObject;
}