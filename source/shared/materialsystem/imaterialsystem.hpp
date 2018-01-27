#ifndef IMATERIALSYSTEM_HPP
#define IMATERIALSYSTEM_HPP

#include "imaterial.hpp"

#include <string>

class IMaterialSystem
{
public:
	virtual ~IMaterialSystem() {}

	virtual bool Init() = 0;

	virtual IMaterial *CreateMaterial( const std::string &materialName ) = 0;
};

#define MATERIALSYSTEM_INTERFACE_VERSION "MATSYSV001"

#endif // IMATERIALSYSTEM_HPP