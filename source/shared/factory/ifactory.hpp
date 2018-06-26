#ifndef IFACTORY_HPP
#define IFACTORY_HPP

#include "string.hpp"
#include "platform.hpp"
#include "iinterface.hpp"

class IFactory
{
public:
	virtual ~IFactory() = default;
	virtual void AddDLLInterface( IDLLInterface *pDLLInterface ) = 0;
	virtual void RemoveDLLInterface( IDLLInterface *pDLLInterface ) = 0;

	virtual void *GetInterface( const string &strInterfaceName ) = 0;
	virtual bool LoadModule( const string &strModule ) = 0;
};

FACTORY_INTERFACE IFactory *GetFactory();

#endif // IFACTORY_HPP