#include "memoryreader.hpp"

#include <stdlib.h>
#include <cstring>

MemoryReader::MemoryReader()
{
	m_pData = nullptr;

	m_Size = 0;
	m_Pos = 0;

	m_bAutoFree = true;
}

MemoryReader::MemoryReader( char *pData, size_t length, bool bAutoFree /*= true*/ ) : MemoryReader()
{
	open( pData, length, bAutoFree );
}

MemoryReader::~MemoryReader()
{
	if ( m_bAutoFree && m_pData != nullptr )
		close_and_free();
}

void MemoryReader::open( char *pData, size_t length, bool bAutoFree /*= true*/ )
{
	if ( m_bAutoFree && m_pData != nullptr )
		close_and_free();
	else
		close();
	
	m_Size = length;

	m_pData = ( char* )malloc( length );
	std::memcpy( ( void* )m_pData, ( void* )pData, length );
	m_bAutoFree = bAutoFree;
}

void MemoryReader::close()
{
	m_Size = 0;
	m_Pos = 0;

	m_pData = nullptr;
}

void MemoryReader::close_and_free()
{
	if ( m_pData != nullptr )
		free( m_pData );

	close();
}

bool MemoryReader::seek( size_t nPos )
{
	if ( nPos >= 0 && nPos < m_Size )
		m_Pos = nPos;
	else
		return false;

	return true;
}

bool MemoryReader::read( char *pDst, size_t count )
{
	// We hit the end of the memory and are trying to read more than we have left
	if ( m_Pos + count > m_Size )
	{
		m_Pos = m_Size;
		return false;
	}

	std::memcpy( ( void* )pDst, ( void* )&m_pData[ m_Pos ], count );
	m_Pos += count;

	return true;
}