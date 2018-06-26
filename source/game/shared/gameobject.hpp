#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include "engine/igameobject.hpp"
#include "renderer/imodel.hpp"
#include "networktable.hpp"

class GameObject : public IGameObject
{
public:
	GameObject();
	virtual ~GameObject() = default;

	bool InitializeGameObject( const string &strModelName = "" ) override;

	virtual void Spawn() override;

	virtual const Vector3f &GetPosition() const override { return m_vPosition; }
	virtual const Vector3f &GetEulerAngles() const override { return m_vEulerAngles; };

	virtual void OnRemove() override;
	virtual void Destroy() override;

	virtual void Think() override;

	virtual void Simulate() override;

	INetObject *GetNetObject() override { return m_pNetObject; }
	INetworkTable *GetNetworkTable() override { return static_cast< INetworkTable* >( &m_NetworkTable ); }

	void SetupNetworkTable();

#ifdef CLIENT_DLL
	virtual void Render();
#endif

protected:
	void SetNetObject( INetObject *pNetObject ) override;

	NetworkTable m_NetworkTable;

private:
	INetObject *m_pNetObject;

	Vector3f m_vPosition;
	Vector3f m_vEulerAngles;

	IModel *m_pModel;

	uint32_t m_iIndex;
};

#endif // GAMEOBJECT_HPP