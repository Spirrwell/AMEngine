#ifndef ISHADER_HPP
#define ISHADER_HPP

#include <stdint.h>
#include <vector>

#include "string.hpp"
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
	string parameterName;
	MaterialParameterType type;
	string defaultValue;
};

class IShader
{
public:
	virtual ~IShader() = default;

	virtual void Initialize() = 0;
	virtual void Update( IMaterial *pMaterial ) = 0;

	virtual const string &GetName() = 0;
	virtual unsigned int GetIndex() = 0;

	virtual std::vector < MaterialParameter_t > GetMaterialParameters() = 0;

	// Used for setting non-opaque uniform types, meaning they are part of a uniform block
	// We're using SPIR-V requires these uniforms to to be set as part of a uniform block
	virtual void SetMatrix3f( const string &uniform, Matrix3f mat3x3 ) = 0;
	virtual void SetMatrix4f( const string &uniform, Matrix4f mat4x4 ) = 0;
	virtual void SetVector2f( const string &uniform, Vector2f vec2 ) = 0;
	virtual void SetVector3f( const string &uniform, Vector3f vec3 ) = 0;
	virtual void SetVector4f( const string &uniform, Vector4f vec4 ) = 0;
	virtual void SetFloat( const string &uniform, float flValue ) = 0;
	virtual void SetInt( const string &uniform, int iValue ) = 0;

	// Used for setting opaque uniform types, which cannot be part of a uniform block
	virtual void SetSampler( const string &uniform, int iValue ) = 0;

	virtual void BindUBO( const string &strBlockName ) = 0;
	virtual void UnbindUBO() = 0;

	virtual void BindShader() = 0;
	virtual void Shutdown() = 0;

	virtual void CreateUniformBlock( const string &strBlockName ) = 0;
	virtual void AddUniformToBlock( const string &strBlockName, const string &strUniformName ) = 0;
	virtual void AddUniform( const string &strUniformName ) = 0;
};

#endif // ISHADER_HPP