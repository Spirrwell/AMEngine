#include "material.hpp"
#include "platform.hpp"
#include "renderer/irenderer.hpp"
#include "renderer/itexture.hpp"
#include "materialsystem.hpp"

#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <sstream>

Material::Material( const string &materialPath )
{
	std::ifstream materialFile( materialPath );

	if ( !materialFile.is_open() )
	{
		printf( "Failed to open material: %s\n", materialPath.c_str() );
		return;
	}

	string line;

	// First obtain the shader by name
	while ( !materialFile.eof() )
	{
		std::getline( materialFile, line );
		std::vector < string > tokens;
		std::istringstream ss( line );
		string token;

		while ( ss >> token )
			if ( token != "=" )
				tokens.push_back( token );

		if ( tokens.size() == 2 )
		{
			if ( tokens[ 0 ] == "ShaderName" )
			{
				tokens[ 1 ].erase( std::remove( tokens[ 1 ].begin(), tokens[ 1 ].end(), '\"' ), tokens[1].end() );
				m_pShader = g_pRenderer->GetShader( tokens[ 1 ] );
				break;
			}
		}
	}

	if ( m_pShader == nullptr )
	{
		materialFile.close();
		printf( "Could not find shader!\n" );
		return;
	}

	m_vMaterialParameters = m_pShader->GetMaterialParameters();

	materialFile.clear();
	materialFile.seekg( 0, std::ios::beg );

	while ( !materialFile.eof() )
	{
		std::getline( materialFile, line );
		std::vector < string > tokens;
		std::istringstream ss( line );
		string token;

		while ( ss >> token ) tokens.push_back( token );

		if ( tokens.size() == 2 )
		{
			if ( tokens[ 0 ].at( 0 ) == '$' )
			{
				tokens[ 0 ].erase( tokens[ 0 ].begin() );
				tokens[ 1 ].erase( std::remove( tokens[ 1 ].begin(), tokens[ 1 ].end(), '\"' ), tokens[ 1 ].end() );

				for ( MaterialParameter_t &matParam : m_vMaterialParameters )
				{
					if ( matParam.parameterName == tokens[ 0 ] )
					{
						switch ( matParam.type )
						{
						case MATP_TEXTURE:
							m_mapTextures[ matParam.parameterName ] = g_pRenderer->CreateTexture( tokens[ 1 ] );
							break;
						case MATP_SKYTEXTURE:
							m_mapTextures[ matParam.parameterName ] = g_pRenderer->CreateTexture( tokens[ 1 ], true );
							break;
						case MATP_FLOAT:
							m_mapFloats[ matParam.parameterName ] = std::stof( tokens[ 1 ] );
							break;
						}
					}
				}

				printf( "parameterName: %s parameterValue: %s\n", tokens[ 0 ].c_str(), tokens[ 1 ].c_str() );
			}
		}
	}

	for ( MaterialParameter_t &matParam : m_vMaterialParameters )
	{
		if ( matParam.type == MATP_TEXTURE && m_mapTextures[ matParam.parameterName ] == nullptr )
			m_mapTextures[ matParam.parameterName ] = g_pRenderer->CreateTexture( matParam.defaultValue );
		else if ( matParam.type == MATP_FLOAT && m_mapFloats.find( matParam.parameterName ) == m_mapFloats.end() )
			m_mapFloats[ matParam.parameterName ] = std::stof( matParam.defaultValue );
	}

	materialFile.close();
}

Material::~Material()
{
	for ( auto &kv : m_mapTextures )
		delete kv.second;
}

void Material::Bind()
{
	m_pShader->BindShader();
	m_pShader->Draw( this );
}

bool Material::IsValid()
{
	return ( m_pShader != nullptr );
}

ITexture *Material::GetTexture( const string &paramName )
{
	return m_mapTextures[ paramName ];
}

float Material::GetFloat( const string &paramName )
{
	return m_mapFloats[ paramName ];
}
