#ifndef IENGINELAUNCHER_HPP
#define IENGINELAUNCHER_HPP

class IEngineLauncher
{
public:
    virtual ~IEngineLauncher() {}

	virtual bool Init() = 0;
	virtual void PostInit() = 0;

	virtual void Shutdown() = 0;
};

#define ENGINELAUNCHER_INTERFACE_VERSION "AMENGINELAUNCHER_VERSION_001"

#endif // IENGINELAUNCHER_HPP