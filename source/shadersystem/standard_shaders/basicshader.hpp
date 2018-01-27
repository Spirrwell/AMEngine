#ifndef BASICSHADER_HPP
#define BASICSHADER_HPP

#include "baseshader.hpp"

namespace BasicShader
{
	struct BasicShader_Vars_t
	{
		int m_nDiffuseTexture;
		int m_nSpecularTexture;
		int m_nShininess;
	};

	static std::vector < CShaderParam * > s_pShaderParams;

	CShaderParam diffuseTexture( "diffuse", "", SHADER_PARAM_TYPE_TEXTURE, &s_pShaderParams );
	CShaderParam specularTexture( "specular", "", SHADER_PARAM_TYPE_TEXTURE, &s_pShaderParams );

	class BasicShader : public CBaseShader
	{
	public:
		using CBaseShader::CBaseShader;

		void InitShaderParams() override;
		void InitVars( BasicShader_Vars_t &info );

		void Draw( IMaterial *pMaterial );
	};
}

#endif // BASICSHADER_HPP