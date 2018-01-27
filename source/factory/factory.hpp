#ifndef FACTORY_HPP
#define FACTORY_HPP

#include "ifactory.hpp"

#include <vector>

#ifdef _WIN32
	#include <windows.h>
#elif defined ( __linux__ )
	#include <dlfcn.h>
#endif // _WIN32

class Module
{
public:
	Module( const std::string &strModulePath );
	~Module();

	bool IsValid() { return m_hDLL != nullptr; };
private:

#ifdef _WIN32
	HINSTANCE m_hDLL;
#elif defined ( __linux__ )
    void *m_hDLL;
#else
	// Should just be a void* for other platforms and using dlopen()
	#error
#endif
};

class Factory : public IFactory
{
public:
	virtual ~Factory();
	virtual void AddDLLInterface( IDLLInterface *pDLLInterface );
	virtual void RemoveDLLInterface( IDLLInterface *pDLLInterface );
	virtual void *GetInterface( const std::string &strInterfaceName );
	virtual bool LoadModule( const std::string &strModule );

private:
	std::vector < IDLLInterface* > m_pInterfaces;

	// TODO: Abstract modules per platform
	std::vector < Module* > m_pModules;
};

#endif // FACTORY_HPP