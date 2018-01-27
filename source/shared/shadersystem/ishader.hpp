#ifndef ISHADER_HPP
#define ISHADER_HPP

#include <string>
#include <stdint.h>
#include <vector>

#include "amlib/transform.hpp"
#include "materialsystem/imaterial.hpp"

enum MaterialParameterType
{
	MATP_TEXTURE,
	MATP_SKYTEXTURE,
	MATP_FLOAT,

	NUM_PARAMETERS,
};

struct MaterialParameter_t
{
	std::string parameterName;
	MaterialParameterType type;
	std::string defaultValue;
};

class IShader
{
public:
    virtual ~IShader() {}

	virtual void Initialize() = 0;
	virtual void Update( IMaterial *pMaterial ) = 0;

	virtual const std::string &GetName() = 0;
	virtual unsigned int GetIndex() = 0;

	virtual std::vector < MaterialParameter_t > GetMaterialParameters() = 0;

	// Used for setting non-opaque uniform types, meaning they are part of a uniform block
	// We're using SPIR-V requires these uniforms to to be set as part of a uniform block
	virtual void SetMatrix3f( const std::string &uniform, Matrix3f mat3x3 ) = 0;
	virtual void SetMatrix4f( const std::string &uniform, Matrix4f mat4x4 ) = 0;
	virtual void SetVector2f( const std::string &uniform, Vector2f vec2 ) = 0;
	virtual void SetVector3f( const std::string &uniform, Vector3f vec3 ) = 0;
	virtual void SetVector4f( const std::string &uniform, Vector4f vec4 ) = 0;
	virtual void SetFloat( const std::string &uniform, float flValue ) = 0;
	virtual void SetInt( const std::string &uniform, int iValue ) = 0;

	// Used for setting opaque uniform types, which cannot be part of a uniform block
	virtual void SetSampler( const std::string &uniform, int iValue ) = 0;

	virtual void BindUBO( const std::string &strBlockName ) = 0;
	virtual void UnbindUBO() = 0;

	virtual void BindShader() = 0;
	virtual void Shutdown() = 0;

	virtual void CreateUniformBlock( const std::string &strBlockName ) = 0;
	virtual void AddUniformToBlock( const std::string &strBlockName, const std::string &strUniformName ) = 0;
	virtual void AddUniform( const std::string &strUniformName ) = 0;
};

#endif // ISHADER_HPP