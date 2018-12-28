#include "materialvk.hpp"
#include "shadervk.hpp"
#include "renderer_vulkan.hpp"
#include "texturevk.hpp"
#include "vulkan_helpers.hpp"

#include <algorithm>
#include <sstream>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

MaterialVK::MaterialVK( const string &materialPath )
{
    std::ifstream materialFile( materialPath );

    if ( !materialFile.is_open() )
    {
        stprintf( "Failed to open material: %s\n", materialPath.c_str() );
        return;
    }

    LoadMaterial( materialFile );
}


MaterialVK::MaterialVK( std::ifstream &material )
{
	LoadMaterial( material );
}

MaterialVK::~MaterialVK()
{
	Shutdown();
}

void MaterialVK::Shutdown()
{
	for ( auto &kv : m_mapTextures )
        delete kv.second;

	m_mapTextures.clear();

	if ( m_vkDescriptorPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( vulkan().device, m_vkDescriptorPool, nullptr );
		m_vkDescriptorPool = VK_NULL_HANDLE;
	}
}

void MaterialVK::LoadMaterial( std::ifstream &material )
{
	string line;

    while ( !material.eof() )
    {
        std::getline( material, line );
        std::vector< string > tokens;
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
                m_pShader = GetVkRenderer_Internal().FindShader( tokens[ 1 ] );
                break;
            }
        }
    }

    if ( m_pShader == nullptr )
    {
		material.close();
        stprintf( "Could not find shader for material.\n" );
        return;
    }

    // Copy material params from shader
    m_MaterialParams = m_pShader->GetMaterialParams();

	material.clear();
	material.seekg( 0, std::ios::beg );

    while ( !material.eof() )
    {
        std::getline( material, line );
        std::vector< string > tokens;
        std::istringstream ss( line );
        string token;

        while ( ss >> token ) tokens.push_back( token );

        if ( tokens.size() == 2 )
        {
            if ( tokens[ 0 ].at( 0 ) == '$' )
            {
                tokens[ 0 ].erase( tokens[ 0 ].begin() );
                tokens[ 1 ].erase( std::remove( tokens[ 1 ].begin(), tokens[ 1 ].end(), '\"' ), tokens[ 1 ].end() );

                for ( auto &matParam : m_MaterialParams )
                {
                    if ( matParam.parameterName == tokens[ 0 ] )
                    {
                        switch( matParam.type )
                        {
                        case MATP_TEXTURE:
                            TextureVK *pTexture = new TextureVK;
                            pTexture->Load( string( GAME_DIR ) + tokens[ 1 ] );
                            m_mapTextures[ matParam.parameterName ] = pTexture;
                            break;
                        }
                    }
                }

                stprintf( "parameterName: %s parameterValue: %s\n", tokens[ 0 ].c_str(), tokens[ 1 ].c_str() );
            }
        }
    }

    for ( auto &matParam : m_MaterialParams )
    {
        if ( matParam.type == MATP_TEXTURE && m_mapTextures[ matParam.parameterName ] == nullptr )
        {
            TextureVK *pTexture = new TextureVK;
            pTexture->Load( string( GAME_DIR ) + matParam.defaultValue );
            m_mapTextures[ matParam.parameterName ] = pTexture;
        }
    }

	material.close();

	m_pShader->createDescriptorPool( *this );
    m_pShader->createDescriptorSets( *this );
}

TextureVK *MaterialVK::GetTexture( const string &matParamName )
{
    auto it = m_mapTextures.find( matParamName );
    if ( it != m_mapTextures.end() )
        return it->second;

    return nullptr;
}