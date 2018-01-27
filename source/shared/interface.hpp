#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "iinterface.hpp"
#include "factory/ifactory.hpp"

class DLLInterfaceOverride : IDLLInterface
{
public:
	DLLInterfaceOverride::DLLInterfaceOverride()
	{
		GetFactory()->AddDLLInterface( this );
	}
	virtual ~DLLInterfaceOverride()
	{
		GetFactory()->RemoveDLLInterface( this );
	}

	virtual const std::string &GetInterfaceName() override { return ""; }
	virtual void *GetInterface() override { return nullptr; }
};

template < class Interface, class Concrete >
class DLLInterface : public IDLLInterface
{
public:
	DLLInterface( const std::string &strInterfaceName ) : m_strInterfaceName( strInterfaceName )
    {
        GetFactory()->AddDLLInterface( this );
    }
    virtual ~DLLInterface()
    {
    	GetFactory()->RemoveDLLInterface( this );
    }

	virtual bool Init() override { return true; }

    const std::string &GetInterfaceName() override { return m_strInterfaceName; }
    void *GetInterface() override { return static_cast< Interface* >( &s_Concrete ); }

	Concrete *GetInternal() { return &s_Concrete; }

private:
    std::string m_strInterfaceName;
	static Concrete s_Concrete;
};

template < class Interface, class Concrete >
Concrete DLLInterface< Interface, Concrete >::s_Concrete;

#endif // INTERFACE_HPP