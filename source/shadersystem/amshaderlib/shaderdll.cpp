#include <cassert>

#include "shaderdll.hpp"
#include "factory/ifactory.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IShaderManager *g_pShaderManager = nullptr;
IRenderer *g_pRenderer = nullptr;

CShaderDLL::CShaderDLL()
{
}

CShaderDLL::~CShaderDLL()
{
}

bool CShaderDLL::Init()
{
	g_pShaderManager = ( IShaderManager* )GetFactory()->GetInterface( SHADERMANAGER_INTERFACE );

	if ( g_pShaderManager == nullptr )
		return false;

	g_pRenderer = ( IRenderer* )GetFactory()->GetInterface( RENDERER_INTERFACE );

	if ( g_pRenderer == nullptr )
		return false;

	for ( IBaseShader *& pShader : m_pShaders )
	{
		if ( !pShader->Init() )
			return false;

		g_pShaderManager->InserShader( pShader );
		g_pRenderer->AddShader( pShader );
	}

	return true;
}

void CShaderDLL::Shutdown()
{
	for ( auto &pShader : m_pShaders )
		pShader->Shutdown();
}

void CShaderDLL::InsertShader( IBaseShader *pShader )
{
	assert( pShader );
	m_pShaders.push_back( pShader );
}

IBaseShader *CShaderDLL::GetShader( size_t iShader )
{
	assert( iShader < m_pShaders.size() );
	return m_pShaders[ iShader ];
}

CShaderDLL *GetShaderDLLInternal()
{
	static CShaderDLL s_ShaderDLL;
	return &s_ShaderDLL;
}