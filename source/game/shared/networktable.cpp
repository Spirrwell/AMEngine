#include "networktable.hpp"
#include "amlib/memoryreader.hpp"


void NetworkTable::UpdateNetworkTable( void *pData, size_t length )
{
	MemoryReader memReader( ( char* )pData, length );

	for ( INetworkVar *pNetVar : m_pNetworkVars )
	{
		size_t size = pNetVar->sizeofElement();
		char *pNetData = ( char* )malloc( size );

		memReader.read( pNetData, size );
		pNetVar->SetData( pNetData );

		free( ( void* )pNetData );
	}
}

void NetworkTable::AddNetworkVar( INetworkVar *pNetVar, size_t numBytes /*= 0*/ )
{
	if ( numBytes > 0 )
		pNetVar->SetSize( numBytes );

	m_pNetworkVars.push_back( pNetVar );
}