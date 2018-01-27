#include "skyshader.hpp"
#include "amshaderlib/shaderdll.hpp"

namespace SkyShader
{
	void SkyShader::InitShaderParams()
	{
		m_pShader->CreateUniformBlock( "MatrixBlock" );

		m_pShader->AddUniformToBlock( "MatrixBlock", "view" );
		m_pShader->AddUniformToBlock( "MatrixBlock", "projection" );

		m_pShader->AddUniform( "skybox" ); // binding 0

		m_vMaterialParameters.push_back( MaterialParameter_t { "skybox", MATP_SKYTEXTURE } );
	}

	void SkyShader::InitVars( SkyShader_Vars_t &info )
	{
		info.m_nDiffuseTexture = diffuseTexture;
		info.m_nSpecularTexture = specularTexture;
	}

	void SkyShader::Draw( IMaterial *pMaterial )
	{
		ITexture *pSkybox = pMaterial->GetTexture( "skybox" );
		pSkybox->Bind( 0, TEXTURE_CUBE_MAP );

		Matrix4f view = Matrix4f( Matrix3f( g_pRenderer->GetViewPort()->GetViewMatrix() ) );
		Matrix4f projection = g_pRenderer->GetViewPort()->GetProjection();

		m_pShader->BindUBO( "MatrixBlock" );

		m_pShader->SetMatrix4f( "view", view );
		m_pShader->SetMatrix4f( "projection", projection );

		m_pShader->UnbindUBO();
	}

	static SkyShader s_SkyShader( "skyShader" );
}