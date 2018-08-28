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
	friend class Factory;
public:
	Module( const string &strModulePath );
	virtual ~Module();

	bool Init();
	void Shutdown();

	inline bool IsValid() { return m_hDLL != nullptr; };

protected:
	void AddInterface( IDLLInterface *pInterface ) { m_pInterfaces.push_back( pInterface ); }
private:

#ifdef _WIN32
	HINSTANCE m_hDLL;
#elif defined ( __linux__ )
    void *m_hDLL;
#else
	// Should just be a void* for other platforms and using dlopen()
	#error
#endif

	std::vector < IDLLInterface* > m_pInterfaces;
};

class Factory : public IFactory
{
public:
	virtual ~Factory();

	virtual void Shutdown();

	virtual void AddDLLInterface( IDLLInterface *pDLLInterface );
	virtual void RemoveDLLInterface( IDLLInterface *pDLLInterface );
	virtual void *GetInterface( const string &strInterfaceName );
	virtual bool LoadModule( const string &strModule );

private:
	std::vector < IDLLInterface* > m_pInterfaces;

	// TODO: Abstract modules per platform
	std::vector < Module* > m_pModules;
};

#endif // FACTORY_HPP