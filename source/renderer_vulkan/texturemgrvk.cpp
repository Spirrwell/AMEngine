#include "texturemgrvk.hpp"
#include "texturevk.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

std::map< string, TextureVK * > TextureMgrVK::m_pTextureMap;
std::vector< TextureVK* > TextureMgrVK::m_pTextures;

void TextureMgrVK::Shutdown()
{
	for ( auto pTexture : m_pTextures )
		delete pTexture;

	m_pTextures.clear();
	m_pTextureMap.clear();
}

TextureVK *TextureMgrVK::LoadTexture( const std::filesystem::path &inpath )
{
	if ( m_pTextureMap[ inpath.string() ] == nullptr )
	{
		auto texture = new TextureVK;
		texture->Load( inpath );
		m_pTextureMap[ inpath.string() ] = texture;
		m_pTextures.push_back( texture );
	}

	return m_pTextureMap[ inpath.string() ];
}

TextureVK *TextureMgrVK::LoadRGBATextureID( const unsigned char *pPixels, size_t width, size_t height, const string &identifier )
{
	if ( m_pTextureMap[ identifier ] == nullptr )
	{
		auto texture = new TextureVK;
		texture->LoadRGBA( pPixels, width, height );
		m_pTextureMap[ identifier ] = texture;
		m_pTextures.push_back( texture );
	}
	return m_pTextureMap[ identifier ];
}

TextureVK *TextureMgrVK::LoadSkyTexture( const std::filesystem::path &inpath )
{
	if ( m_pTextureMap[ inpath.string() ] == nullptr )
	{
		auto texture = new TextureVK;
		texture->LoadKtx( inpath );
		m_pTextureMap[ inpath.string() ] = texture;
		m_pTextures.push_back( texture );
	}

	return m_pTextureMap[ inpath.string() ];
}