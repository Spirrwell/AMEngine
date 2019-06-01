#ifndef MEMORYOVERRIDE_HPP
#define MEMORYOVERRIDE_HPP

#include <cstdlib>

#define MEM_DEBUGGING 0

#if MEM_DEBUGGING
void *debug_malloc( size_t size, const char *pszFile, int line, const char *pszFunction );
void debug_free( void *pMem );
void *debug_realloc( void *pMem, size_t size, const char *pszFile, int line, const char *pszFunction );
void *debug_aligned_malloc( size_t size, size_t align, const char *pszFile, int line, const char *pszFunction );
void debug_aligned_free( void *pMem );

// C++ new and delete
void *operator new( std::size_t size, const char *pszFile, int line, const char *pszFunction );
void *operator new[]( std::size_t size, const char *pszFile, int line, const char *pszFunction );

void operator delete( void *pMem, std::size_t size );
void operator delete( void *pMem );
void operator delete( void *pMem, const char *pszFile, int line, const char *pszFunction );
void operator delete( void *pMem, std::size_t size, const char *pszFile, int line, const char *pszFunction );
void operator delete[]( void *pMem );
void operator delete[]( void *pMem, std::size_t size );
void operator delete[]( void *pMem, const char *pszFile, int line, const char *pszFunction );
void operator delete[]( void *pMem, std::size_t size, const char *pszFile, int line, const char *pszFunction );

// C++17 Over aligned allocation
void *operator new( std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction );
void *operator new[]( std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction );

void operator delete( void *pMem, std::size_t size, std::align_val_t align );
void operator delete( void *pMem, std::align_val_t align );
void operator delete( void *pMem, std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction );
void operator delete( void *pMem, std::align_val_t align, const char *pszFile, int line, const char *pszFunction );
void operator delete[]( void *pMem, std::size_t size, std::align_val_t align );
void operator delete[]( void *pMem, std::align_val_t align );
void operator delete[]( void *pMem, std::size_t size, std::align_val_t align, const char *pszFile, int line, const char *pszFunction );
void operator delete[]( void *pMem, std::align_val_t align, const char *pszFile, int line, const char *pszFunction );

#define malloc(X) debug_malloc( X, __FILE__, __LINE__, __FUNCTION__ )
#define free(X) debug_free( X )
#define realloc(X, Y) debug_realloc( X, Y, __FILE__, __LINE__, __FUNCTION__ )
#define _aligned_malloc(X, Y) debug_aligned_malloc( X, Y, __FILE__, __LINE__, __FUNCTION__ )
#define _aligned_free(X) debug_aligned_free( X )

#define DBG_NEW new ( __FILE__, __LINE__, __FUNCTION__ )
#define new DBG_NEW

#endif

#endif // MEMORYOVERIDE_HPP