#include "genericshader.hpp"

void GenericShader::Initialize()
{
	ShaderGL::Initialize();
	LinkAndValidate();

	AddUniform( "diffuse" ); // binding 0
	AddUniform( "specular" ); // binding 1

	CreateUniformBlock( "UBO" );
	CreateUniformBlock( "UBO_FS" );

	AddUniformToBlock( "UBO", "projection" );
	AddUniformToBlock( "UBO", "view" );
	AddUniformToBlock( "UBO", "model" );
	AddUniformToBlock( "UBO", "normalMatrix" );

	AddUniformToBlock( "UBO_FS", "light.position" );
	AddUniformToBlock( "UBO_FS", "light.ambient" );
	AddUniformToBlock( "UBO_FS", "light.diffuse" );
	AddUniformToBlock( "UBO_FS", "light.specular" );
	AddUniformToBlock( "UBO_FS", "camPos" );
	AddUniformToBlock( "UBO_FS", "shininess" );

	m_vMaterialParameters.push_back( MaterialParameter_t { "diffuse", MATP_TEXTURE, "textures/shader/error.png" } );
	m_vMaterialParameters.push_back( MaterialParameter_t { "specular", MATP_TEXTURE, "textures/shader/white.png" } );
	m_vMaterialParameters.push_back( MaterialParameter_t { "shininess", MATP_FLOAT, "32.0" } );
}

void GenericShader::Update( IMaterial *pMaterial )
{
	//SetSampler( "diffuse", 0 );
	//SetSampler( "specular", 1 );

	pMaterial->GetTexture( "diffuse" )->Bind( 0, GL_TEXTURE_2D );
	pMaterial->GetTexture( "specular" )->Bind( 1, GL_TEXTURE_2D );

	Matrix4f projection = g_pRenderer->GetViewPort()->GetProjection();
	Matrix4f view = g_pRenderer->GetViewPort()->GetViewMatrix();

	BindUBO( "UBO" );

	SetMatrix4f( "projection", projection );
	SetMatrix4f( "view", view );

	Vector3f lightPos = Vector3f( 0.0f, 20.0f, -5.0f );
	Vector3f lightAmbient = Vector3f( 0.2f, 0.2f, 0.2f );
	Vector3f lightDiffuse = Vector3f( 0.5f, 0.5f, 0.5f );
	Vector3f lightSpecular = Vector3f( 1.0f, 1.0f, 1.0f );
	Vector3f camPos = g_pRenderer->GetViewPort()->GetCamera()->GetPosition();
	float shininess = pMaterial->GetFloat( "shininess" );

	BindUBO( "UBO_FS" );

	SetVector3f( "light.position", lightPos );
	SetVector3f( "light.ambient", lightAmbient );
	SetVector3f( "light.diffuse", lightDiffuse );
	SetVector3f( "light.specular", lightSpecular );
	SetVector3f( "camPos", camPos );
	SetFloat( "shininess", shininess );

	UnbindUBO();
}
