#include "imemory_system.hpp"

#include "string.hpp"
#include <map>

struct Allocation_t
{
	size_t size; // Allocation size
	string fileName; // Where were we allocated
	int line; // Line where we were allocated
	string functionName; // Name of function where we were allocated
};

class CMemTracker : public IMemTracker
{
public:
	CMemTracker();

	void *Malloc( size_t size, const char *pszFile, int line, const char *pszFunction ) override;
	void *Malloc( size_t size ) override;
	void Free( void *pMem ) override;
	void *ReAlloc( void *pMem, size_t size, const char *pszFile, int line, const char *pszFunction ) override;
	void *ReAlloc( void *pMem, size_t size ) override;
	void *AlignedMalloc( size_t size, size_t align, const char *pszFile, int line, const char *pszFunction ) override;
	void *AlignedMalloc( size_t size, size_t align ) override;
	void AlignedFree( void *pMem ) override;

	void PrintAllocations();

private:

	void *InsertAllocation( void *pMem, size_t size, const char *pszFile = "Unknown", int line = -1, const char *pszFunction = "Unknown" );
	void RemoveAllocation( void *pMem );

	std::map< void*, Allocation_t > m_mapAllocations;

	uint64_t allocations;
	uint64_t deallocations;
};