#ifndef INETWORKVAR_HPP
#define INETWORKVAR_HPP

#include <cstdlib>

class INetworkVar
{
	friend class NetworkTable;

public:
	virtual void *data() = 0;
	virtual size_t sizeofElement() = 0;

	virtual bool StateChanged() = 0;

protected:
	virtual void SetData( char *pData ) = 0;
	virtual void SetSize( size_t size ) = 0;
};

#endif // INETWORKVAR_HPP