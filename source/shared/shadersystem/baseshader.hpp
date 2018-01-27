#ifndef BASESHADER_HPP
#define BASESHADER_HPP

#include <string>

#include "ishader.hpp"
#include "ibaseshader.hpp"

enum ShaderMaterialVars_t
{
	BASETEXTURE,
	SPECUALR,

	NUM_SHADER_MATERIAL_VARS
};

enum ShaderParamType_t
{
	SHADER_PARAM_TYPE_TEXTURE,
	SHADER_PARAM_TYPE_FLOAT,
};

class CShaderParam
{
public:
	CShaderParam( ShaderMaterialVars_t var, ShaderParamType_t type, const std::string &strDefaultValue, CShaderParam **pShaderParamsOverrides[ NUM_SHADER_MATERIAL_VARS ] )
	{
		( *pShaderParamsOverrides )[ var ] = this;
		m_iIndex = var;
	}
	CShaderParam( const std::string &strName, const std::string &strDefaultValue, ShaderParamType_t type, std::vector < CShaderParam * > *pShaderParams )
	{
		m_iIndex = ( int )pShaderParams->size();
		pShaderParams->push_back( this );
	}

	operator int()
	{
		return m_iIndex;
	}

	operator std::string()
	{
		return m_strName;
	}

private:
	int m_iIndex;
	std::string m_strName;
};

class CBaseShader : public IBaseShader
{
public:
	CBaseShader( const std::string &strShaderName );

	bool Init();
	virtual void InitShaderParams() {}
	virtual void Draw( IMaterial *pMaterial ) {}

	void BindShader() { m_pShader->BindShader(); }
	void Shutdown() { m_pShader->Shutdown(); }

	IShader *GetIShader() { return m_pShader; }

	std::vector < MaterialParameter_t > GetMaterialParameters() { return m_vMaterialParameters; };

	const std::string &GetName() { return m_strShaderName; }

protected:

	std::string m_strShaderName;
	IShader *m_pShader;
	std::vector < MaterialParameter_t > m_vMaterialParameters;
};

#endif // BASESHADER_HPP