#include "memorywriter.hpp"

#include <stdlib.h>
#include <cstring>

MemoryWriter::MemoryWriter( bool bAutoFree /*= true*/ )
{
	m_pData = nullptr;
	m_Size = 0;
	m_bAutoFree = bAutoFree;
}

MemoryWriter::MemoryWriter( char *pData, size_t length, bool bAutoFree /*= true*/ )
{
	m_Size = length;
	m_bAutoFree = bAutoFree;

	m_pData = ( char* )malloc( m_Size );
	std::memcpy( m_pData, pData, m_Size );
}

MemoryWriter::~MemoryWriter()
{
	if ( m_bAutoFree && m_pData != nullptr )
		close_and_free();
}

void MemoryWriter::close()
{
	m_Size = 0;
	m_pData = nullptr;
}

void MemoryWriter::close_and_free()
{
	if ( m_pData != nullptr )
		free( m_pData );

	close();
}

void MemoryWriter::write( char *pSrc, size_t length )
{
	// This is where we start writing
	size_t offset = m_Size;

	m_Size += length;
	m_pData = ( m_pData == nullptr ) ? ( char* )malloc( length ) : ( char* )realloc( m_pData, m_Size );

	std::memcpy( ( void* )&m_pData[ offset ], ( void* )pSrc, length );
}