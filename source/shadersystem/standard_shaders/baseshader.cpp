#include "baseshader.hpp"
#include "amshaderlib/shaderdll.hpp"

CBaseShader::CBaseShader( const std::string &strShaderName )
{
	m_strShaderName = strShaderName;

	GetShaderDLLInternal()->InsertShader( this );
}

bool CBaseShader::Init()
{
	m_pShader = g_pShaderManager->CreateShaderObject( m_strShaderName );

	if ( m_pShader == nullptr )
		return false;

	return true;
}