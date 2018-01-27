#ifndef IMATERIAL_HPP
#define IMATERIAL_HPP

#include <string>

#include "renderer/itexture.hpp"
#include "amlib/transform.hpp"

class IBaseShader;

class IMaterial
{
public:
    virtual ~IMaterial() {}

	virtual void Bind() = 0;
	virtual bool IsValid() = 0;

	virtual ITexture *GetTexture( const std::string &paramName ) = 0;
	virtual IBaseShader *GetShader() = 0;

	virtual float GetFloat( const std::string &paramName ) = 0;
};

#endif // IMATERIAL_HPP