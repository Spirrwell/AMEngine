#ifndef IFACTORY_HPP
#define IFACTORY_HPP

#include <string>

#include "platform.hpp"
#include "iinterface.hpp"

class IFactory
{
public:
	virtual ~IFactory() {};
	virtual void AddDLLInterface( IDLLInterface *pDLLInterface ) = 0;
	virtual void RemoveDLLInterface( IDLLInterface *pDLLInterface ) = 0;

	virtual void *GetInterface( const std::string &strInterfaceName ) = 0;
	virtual bool LoadModule( const std::string &strModule ) = 0;
};

FACTORY_INTERFACE IFactory *GetFactory();

#endif // IFACTORY_HPP