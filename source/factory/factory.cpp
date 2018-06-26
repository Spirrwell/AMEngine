#include <cassert>

#include "factory.hpp"

Module::Module( const std::string &strModulePath )
{
	m_hDLL = nullptr;
#ifdef _WIN32
	m_hDLL = ::LoadLibraryA( strModulePath.c_str() );
#elif defined ( __linux__ )
	m_hDLL = dlopen( strModulePath.c_str(), RTLD_LAZY );
	if ( !IsValid() )
		printf( "Error: %s\n", dlerror() );
#endif // _WIN32
}

Module::~Module()
{
	if ( IsValid() )
#ifdef _WIN32
		::FreeLibrary( m_hDLL );
#elif defined ( __linux__ )
        if ( dlclose( m_hDLL ) != 0 )
			printf( "Error: %s\n", dlerror() );
#endif
}

Factory::~Factory()
{
	for ( auto &pModule : m_pModules )
		delete pModule;

	m_pModules.clear();
}

void Factory::AddDLLInterface( IDLLInterface *pDLLInterface )
{
	assert( pDLLInterface != nullptr );
	m_pInterfaces.push_back( pDLLInterface );
}

void Factory::RemoveDLLInterface( IDLLInterface *pDLLInterface )
{
	assert( pDLLInterface != nullptr );
	for ( auto it = m_pInterfaces.begin(); it != m_pInterfaces.end(); it++ )
	{
		if ( *it == pDLLInterface )
		{
			m_pInterfaces.erase( it );
			return;
		}
	}
}

void *Factory::GetInterface( const std::string &strInterfaceName )
{
	// If an interface has no name, we shouldn't be trying to get it
	if ( strInterfaceName.empty() )
		return nullptr;

	// There should only ever be one interface with a particular name
	for ( IDLLInterface *pDLLInterface : m_pInterfaces )
		if ( pDLLInterface->GetInterfaceName() == strInterfaceName )
			return pDLLInterface->GetInterface();

	return nullptr;
}

bool Factory::LoadModule( const std::string &strModule )
{
	size_t oldInterfaceCount = m_pInterfaces.size();
	Module *pModule = new Module( strModule );
	
	if ( !pModule->IsValid() )
	{
		delete pModule;
		return false;
	}

	size_t newInterfaceCount = m_pInterfaces.size();

	if ( newInterfaceCount > oldInterfaceCount )
	{
		for ( size_t i = oldInterfaceCount; i < newInterfaceCount; i++ )
		{
			if ( !m_pInterfaces[ i ]->Init() )
			{
				delete pModule;
				return false;
			}
		}
	}

	m_pModules.push_back( pModule );

	return true;
}

static Factory s_Factory;
IFactory *GetFactory() { return static_cast< IFactory* >( &s_Factory ); }
