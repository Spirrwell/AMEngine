#ifndef SKYSHADER_H
#define SKYSHADER_H

#include "baseshader.hpp"

namespace SkyShader
{
	struct SkyShader_Vars_t
	{
		int m_nDiffuseTexture;
		int m_nSpecularTexture;
		int m_nShininess;
	};

	static std::vector < CShaderParam * > s_pShaderParams;
	CShaderParam diffuseTexture( "diffuse", "", SHADER_PARAM_TYPE_TEXTURE, &s_pShaderParams );
	CShaderParam specularTexture( "specular", "", SHADER_PARAM_TYPE_TEXTURE, &s_pShaderParams );

	class SkyShader : public CBaseShader
	{
	public:
		using CBaseShader::CBaseShader;

		void InitShaderParams() override;
		void InitVars( SkyShader_Vars_t &info );

		void Draw( IMaterial *pMaterial );
	};
}

#endif // SKYSHADER_H