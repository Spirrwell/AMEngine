#include "server.hpp"
#include "interface.hpp"
#include "gameobjectfactory.hpp"

bool ServerDLL::Init()
{
	return true;
}

IGameObjectFactory *ServerDLL::GetGameObjectFactory()
{
	return ::GetGameObjectFactory();
}

static DLLInterface < IServerDLL, ServerDLL > s_ServerDLL( SERVER_INTERFACE_VERSION );