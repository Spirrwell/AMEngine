#ifndef ENGINELAUNCHER_HPP
#define ENGINELAUNCHER_HPP

#include "ienginelauncher.hpp"

class EngineLauncher : public IEngineLauncher
{
public:
	virtual ~EngineLauncher();

	virtual bool Init();
	virtual void PostInit();

	virtual void Shutdown();
};

#endif // ENGINELAUNCHER_HPP