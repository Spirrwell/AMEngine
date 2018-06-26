#ifndef IMATERIALSYSTEM_HPP
#define IMATERIALSYSTEM_HPP

#include "imaterial.hpp"
#include "string.hpp"

class IMaterialSystem
{
public:
	virtual ~IMaterialSystem() = default;

	virtual bool Init() = 0;

	virtual IMaterial *CreateMaterial( const string &materialName ) = 0;
};

#define MATERIALSYSTEM_INTERFACE_VERSION "MATSYSV001"

#endif // IMATERIALSYSTEM_HPP