#ifndef IMEMORY_SYSTEM_HPP
#define IMEMORY_SYSTEM_HPP

#include <stdlib.h>
#include "platform.hpp"

class IMemTracker
{
public:
	virtual ~IMemTracker() = default;

	virtual void *Malloc( size_t size, const char *pszFile, int line, const char *pszFunction ) = 0;
	virtual void *Malloc( size_t size ) = 0;
	virtual void Free( void *pMem ) = 0;
	virtual void *ReAlloc( void *pMem, size_t size, const char *pszFile, int line, const char *pszFunction ) = 0;
	virtual void *ReAlloc( void *pMem, size_t size ) = 0;
	virtual void *AlignedMalloc( size_t size, size_t align, const char *pszFile, int line, const char *pszFunction ) = 0;
	virtual void *AlignedMalloc( size_t size, size_t align ) = 0;
	virtual void AlignedFree( void *pMem ) = 0;

	virtual void PrintAllocations() = 0;
};

MEMORY_SYSTEM_INTERFACE IMemTracker *GetMemTracker();

#endif // IMEMORY_SYSTEM_HPP