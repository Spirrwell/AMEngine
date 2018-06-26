#include "gameobject.hpp"

GameObject::GameObject()
{
	m_pModel = nullptr;
	m_pNetObject = nullptr;

	m_iIndex = 0;
}

bool GameObject::InitializeGameObject( const string &strModelName /*= ""*/ )
{
	if ( !strModelName.empty() )
	{
		// Load model here
	}

	return true;
}

void GameObject::Spawn()
{

}

void GameObject::OnRemove()
{

}

void GameObject::Destroy()
{

}

void GameObject::Think()
{

}

void GameObject::Simulate()
{

}

void GameObject::SetupNetworkTable()
{

}

#ifdef CLIENT_DLL
void GameObject::Render()
{
	if ( m_pModel != nullptr )
	{
		m_pModel->Draw();
	}
}
#endif

void GameObject::SetNetObject( INetObject *pNetObject )
{
	m_pNetObject = pNetObject;
	m_iIndex = m_pNetObject->GetNetIndex();
}