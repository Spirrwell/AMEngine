#ifndef IGAMEOBJECT_HPP
#define IGAMEOBJECT_HPP

#include "mathdefs.hpp"
#include "engine/inetobject.hpp"
#include "engine/inetworktable.hpp"
#include "string.hpp"

class IGameObjectRegister;

class IGameObject
{
	friend struct NetObject;

public:
	virtual ~IGameObject() = default;

	virtual bool InitializeGameObject( const string &strModelName = "" ) = 0;

	virtual void Spawn() = 0;

	virtual const Vector3f &GetPosition() const = 0;
	virtual const Vector3f &GetEulerAngles() const = 0;

	virtual void OnRemove() = 0;
	virtual void Destroy() = 0;

	virtual void Think() = 0;

	virtual void Simulate() = 0;

	virtual IGameObjectRegister *GetRegister() = 0;

	virtual INetObject *GetNetObject() = 0;
	virtual INetworkTable *GetNetworkTable() = 0;

	virtual void SetupNetworkTable() = 0;

protected:
	virtual void SetNetObject( INetObject *pNetObject ) = 0;
};

#endif //IGAMEOBJECT_HPP