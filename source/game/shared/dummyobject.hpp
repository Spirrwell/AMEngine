#ifndef DUMMYOBJECT_HPP
#define DUMMYOBJECT_HPP

#include "gameobject.hpp"
#include "networktable.hpp"
#include "gameobjectfactory.hpp"

class DummyObject : public GameObject
{
public:
	DummyObject();
	~DummyObject();

	void Think() override;

	void SetupNetworkTable();
	IGameObjectRegister *GetRegister() { return &gameobj_register; }

protected:
	static GameObjectRegister < DummyObject > gameobj_register;

private:
	NetworkVar < int > m_iTest;
};

#endif // DUMMYOBJECT_HPP