#include "dummyobject.hpp"

DummyObject::DummyObject()
{
#ifdef SERVER_DLL
	m_iTest = 777;
#else
	m_iTest = 42;
#endif
}

DummyObject::~DummyObject()
{
}

void DummyObject::Think()
{
#ifdef CLIENT_DLL
	printf( "DummyObject: %d\n", m_iTest.Get() );
#endif
}

void DummyObject::SetupNetworkTable()
{
	m_NetworkTable.AddNetworkVar( &m_iTest, 4 );
	//m_NetTable.AddNetworkVar( this, &m_iTest );
}

GameObjectRegister < DummyObject > DummyObject::gameobj_register( "dummy_object" );