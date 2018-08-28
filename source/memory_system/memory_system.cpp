#include "memory_system.hpp"

CMemTracker::CMemTracker()
{
	allocations = 0;
	deallocations = 0;
}

// NOTE: Should we be throwing std::bad_alloc for normal malloc calls?
void *CMemTracker::Malloc( size_t size, const char *pszFile, int line, const char *pszFunction )
{
	void *pMem = malloc( size );
	return pMem ? InsertAllocation( pMem, size, pszFile, line, pszFunction ) : throw std::bad_alloc{};
}

void *CMemTracker::Malloc( size_t size )
{
	void *pMem = malloc( size );
	return pMem ? InsertAllocation( pMem, size ) : throw std::bad_alloc{};
}

void CMemTracker::Free( void *pMem )
{
	RemoveAllocation( pMem );
	return free( pMem );
}

void *CMemTracker::ReAlloc( void *pMem, size_t size, const char *pszFile, int line, const char *pszFunction )
{
	RemoveAllocation( pMem );
	pMem = realloc( pMem, size );

	return pMem ? InsertAllocation( pMem, size, pszFile, line, pszFunction ) : throw std::bad_alloc{};
}

void *CMemTracker::ReAlloc( void *pMem, size_t size )
{
	RemoveAllocation( pMem );
	pMem = realloc( pMem, size );

	return pMem ? InsertAllocation( pMem, size ) : throw std::bad_alloc{};
}

void *CMemTracker::AlignedMalloc( size_t size, size_t align, const char *pszFile, int line, const char *pszFunction )
{
	void *pMem = _aligned_malloc( size, align );
	return pMem ? InsertAllocation( pMem, size, pszFile, line, pszFunction ) : throw std::bad_alloc{};
}

void *CMemTracker::AlignedMalloc( size_t size, size_t align )
{
	void *pMem = _aligned_malloc( size, align );
	return pMem ? InsertAllocation( pMem, size ) : throw std::bad_alloc{};
}

void CMemTracker::AlignedFree( void *pMem )
{
	RemoveAllocation( pMem );
	return _aligned_free( pMem );
}

void CMemTracker::PrintAllocations()
{
	for ( auto it : m_mapAllocations )
	{
		stprintf( "Allocation: %s, %i, %s, %p[%li]\n", 
			it.second.fileName.c_str(), it.second.line, it.second.functionName.c_str(), it.first, it.second.size );
	}

	stprintf( "Allocations: %llu\nDeallocations: %llu\n", allocations, deallocations );
}

void *CMemTracker::InsertAllocation( void *pMem, size_t size, const char *pszFile /*= "Unknown"*/, int line /*= -1*/, const char *pszFunction /*= "Unknown"*/ )
{
	m_mapAllocations[ pMem ] = { size, pszFile, line, pszFunction };
	++allocations;

	return pMem;
}

void CMemTracker::RemoveAllocation( void *pMem )
{
	for ( auto it = m_mapAllocations.cbegin(); it != m_mapAllocations.cend(); ++it )
	{
		if ( ( *it ).first == pMem )
		{
			m_mapAllocations.erase( it );
			++deallocations;
			break;
		}
	}
}

static CMemTracker s_MemTracker;
IMemTracker *GetMemTracker() { return static_cast< IMemTracker* >( &s_MemTracker ); }