#include "server.hpp"
#include "interface.hpp"
#include "gameobjectfactory.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

bool ServerDLL::Init()
{
	return true;
}

IGameObjectFactory *ServerDLL::GetGameObjectFactory()
{
	return ::GetGameObjectFactory();
}

static DLLInterface < IServerDLL, ServerDLL > s_ServerDLL( SERVER_INTERFACE_VERSION );