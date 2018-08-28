#include "baseshader.hpp"
#include "amshaderlib/shaderdll.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

CBaseShader::CBaseShader( const string &strShaderName )
{
	m_pShader = nullptr;
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

void CBaseShader::Shutdown()
{
	m_pShader->Shutdown();
	g_pShaderManager->DeleteShaderObject( m_pShader );

	m_pShader = nullptr;
}