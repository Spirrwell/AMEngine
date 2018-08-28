#include "memory_system/imemory_system.hpp"
#include "string.hpp"

void *debug_malloc( size_t size, const char *pszFile, int line, const char *pszFunction )
{
	return GetMemTracker()->Malloc( size, pszFile, line, pszFunction );
}

void debug_free( void *pMem )
{
	return GetMemTracker()->Free( pMem );
}

void *debug_realloc( void *pMem, size_t size, const char *pszFile, int line, const char *pszFunction )
{
	return GetMemTracker()->ReAlloc( pMem, size, pszFile, line, pszFunction );
}

void *debug_aligned_malloc( size_t size, size_t align, const char *pszFile, int line, const char *pszFunction )
{
	return GetMemTracker()->AlignedMalloc( size, align, pszFile, line, pszFunction );
}

void debug_aligned_free( void *pMem )
{
	return GetMemTracker()->AlignedFree( pMem );
}

void *operator new( std::size_t size, const char *pszFile, int line, const char *pszFunction )
{
	return debug_malloc( size, pszFile, line, pszFunction );
}

void *operator new[]( std::size_t size, const char *pszFile, int line, const char *pszFunction )
{
	return debug_malloc( size, pszFile, line, pszFunction );
}

void operator delete( void *pMem, std::size_t size )
{
	return debug_free( pMem );
}

void operator delete( void *pMem )
{
	return debug_free( pMem );
}

void operator delete( void *pMem, const char *pszFile, int line, const char *pszFunction )
{
	return debug_free( pMem );
}

void operator delete( void *pMem, std::size_t size, const char *pszFile, int line, const char *pszFunction )
{
	return debug_free( pMem );
}

void operator delete[]( void *pMem )
{
	return debug_free( pMem );
}

void operator delete[]( void *pMem, std::size_t size )
{
	return debug_free( pMem );
}

void operator delete[]( void *pMem, const char *pszFile, int line, const char *pszFunction )
{
	debug_free( pMem );
}

void operator delete[]( void *pMem, std::size_t size, const char *pszFile, int line, const char *pszFunction )
{
	debug_free( pMem );
}

// Over aligned allocation
void *operator new( std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction )
{
	return debug_aligned_malloc( size, static_cast< size_t >( align ), pszFile, line, pszFunction );
}

void *operator new[]( std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction )
{
	return debug_aligned_malloc( size, static_cast< size_t >( align ), pszFile, line, pszFunction );
}

void operator delete( void *pMem, std::size_t size, std::align_val_t align )
{
	return debug_aligned_free( pMem );
}

void operator delete( void *pMem, std::align_val_t align )
{
	return debug_aligned_free( pMem );
}

void operator delete( void *pMem, std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction )
{
	return debug_aligned_free( pMem );
}

void operator delete( void *pMem, std::align_val_t align, const char *pszFile, int line, const char *pszFunction )
{
	return debug_aligned_free( pMem );
}

void operator delete[]( void *pMem, std::size_t size, std::align_val_t align )
{
	debug_aligned_free( pMem );
}

void operator delete[]( void *pMem, std::align_val_t align )
{
	debug_aligned_free( pMem );
}

void operator delete[]( void *pMem, std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction )
{
	debug_aligned_free( pMem );
}

void operator delete[]( void *pMem, std::align_val_t align, const char *pszFile, int line, const char *pszFunction )
{
	debug_aligned_free( pMem );
}