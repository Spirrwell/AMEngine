#include "basicshader.hpp"
#include "amshaderlib/shaderdll.hpp"

namespace BasicShader
{
	void BasicShader::InitShaderParams()
	{
		m_pShader->AddUniform( "diffuse" );
		m_pShader->AddUniform( "specular" );

		m_pShader->CreateUniformBlock( "UBO" );
		m_pShader->CreateUniformBlock( "UBO_FS" );

		m_pShader->AddUniformToBlock( "UBO", "projection" );
		m_pShader->AddUniformToBlock( "UBO", "view" );
		m_pShader->AddUniformToBlock( "UBO", "model" );
		m_pShader->AddUniformToBlock( "UBO", "normalMatrix" );

		m_pShader->AddUniformToBlock( "UBO_FS", "light.position" );
		m_pShader->AddUniformToBlock( "UBO_FS", "light.ambient" );
		m_pShader->AddUniformToBlock( "UBO_FS", "light.diffuse" );
		m_pShader->AddUniformToBlock( "UBO_FS", "light.specular" );
		m_pShader->AddUniformToBlock( "UBO_FS", "camPos" );
		m_pShader->AddUniformToBlock( "UBO_FS", "shininess" );

		m_vMaterialParameters.push_back( MaterialParameter_t { "diffuse", MATP_TEXTURE, "textures/shader/error.png" } );
		m_vMaterialParameters.push_back( MaterialParameter_t { "specular", MATP_TEXTURE, "textures/shader/white.png" } );
		m_vMaterialParameters.push_back( MaterialParameter_t { "shininess", MATP_FLOAT, "32.0" } );
	}

	void BasicShader::InitVars( BasicShader_Vars_t &info )
	{
		info.m_nDiffuseTexture = diffuseTexture;
		info.m_nSpecularTexture = specularTexture;
	}

	void BasicShader::Draw( IMaterial *pMaterial )
	{
		pMaterial->GetTexture( "diffuse" )->Bind( 0, TEXTURE_2D );
		pMaterial->GetTexture( "specular" )->Bind( 1, TEXTURE_2D );

		Matrix4f projection = g_pRenderer->GetViewPort()->GetProjection();
		Matrix4f view = g_pRenderer->GetViewPort()->GetViewMatrix();

		m_pShader->BindUBO( "UBO" );

		m_pShader->SetMatrix4f( "projection", projection );
		m_pShader->SetMatrix4f( "view", view );

		Vector3f lightPos = Vector3f( 0.0f, 20.0f, -5.0f );
		Vector3f lightAmbient = Vector3f( 0.2f, 0.2f, 0.2f );
		Vector3f lightDiffuse = Vector3f( 0.5f, 0.5f, 0.5f );
		Vector3f lightSpecular = Vector3f( 1.0f, 1.0f, 1.0f );
		Vector3f camPos = g_pRenderer->GetViewPort()->GetCamera()->GetPosition();
		float shininess = pMaterial->GetFloat( "shininess" );

		m_pShader->BindUBO( "UBO_FS" );

		m_pShader->SetVector3f( "light.position", lightPos );
		m_pShader->SetVector3f( "light.ambient", lightAmbient );
		m_pShader->SetVector3f( "light.diffuse", lightDiffuse );
		m_pShader->SetVector3f( "light.specular", lightSpecular );
		m_pShader->SetVector3f( "camPos", camPos );
		m_pShader->SetFloat( "shininess", shininess );

		m_pShader->UnbindUBO();
	}

	static BasicShader s_BasicShader( "basicShader" );
}