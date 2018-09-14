#ifndef PLATFORM_H
#define PLATFORM_H

#if defined ( _MSC_VER )
	#define DLL_EXPORT __declspec( dllexport )
	#define DLL_IMPORT __declspec( dllimport )
#elif defined ( __GNUC__ )
	#define DLL_EXPORT __attribute__( ( visibility ( "default" ) ) )
	#define DLL_IMPORT
#else
	// Not implemented for other compilers
	#error
#endif // _MSC_VER

#ifdef FACTORY_DLL_EXPORT
	#define FACTORY_INTERFACE DLL_EXPORT
#else
	#define FACTORY_INTERFACE DLL_IMPORT
#endif

#ifdef MEMORY_SYSTEM_DLL_EXPORT
	#define MEMORY_SYSTEM_INTERFACE DLL_EXPORT
#else
	#define MEMORY_SYSTEM_INTERFACE DLL_IMPORT
#endif

#ifdef SDL_CORE_DLL_EXPORT
	#define SDL_CORE_INTERFACE DLL_EXPORT
#else
	#define SDL_CORE_INTERFACE DLL_IMPORT
#endif

// Note: We use std::strings to load our modules, but for the sake of potential future use, these will be C strings for now
#ifdef _WIN32
	#define DLL_EXTENSION ".dll"
#elif defined ( __APPLE__ )
	#define DLL_EXTENSION ".dylib"
#elif defined ( __linux__ )
	#define DLL_EXTENSION ".so"
#else
	// Needs setup for other platforms
	#error
#endif

// Temporary game directory definition
#define GAME_DIR "../../game_resource/"


#endif // PLATFORM_H
