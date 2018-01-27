#include "texturegl.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <map>

static std::map < TextureType, GLenum > g_TextureTypeMap =
{
	{ TEXTURE_2D, GL_TEXTURE_2D },
	{ TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP }
};

TextureGL::TextureGL( const std::string &fileName )
{
	m_bCubeMap = false;
	m_hTextureID = 0;

	int width, height, numComponents;
	unsigned char *pImageData = stbi_load( fileName.c_str(), &width, &height, &numComponents, 4 );

	if ( pImageData == nullptr )
	{
		printf( "Failed to load texture: %s\n", fileName.c_str() );
		return;
	}

	glGenTextures( 1, &m_hTextureID );
	glBindTexture( GL_TEXTURE_2D, m_hTextureID );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	stbi_image_free( pImageData );
}

TextureGL::TextureGL( std::vector < std::string > faces )
{
	m_bCubeMap = true;

	glGenTextures( 1, &m_hTextureID );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_hTextureID );

	int width, height, numComponents;
	for ( unsigned int i = 0; i < faces.size(); i++ )
	{
		unsigned char *pImageData = stbi_load( faces[i].c_str(), &width, &height, &numComponents, 4 );

		if ( pImageData == nullptr )
		{
			printf( "Failed to load texture: %s\n", faces[i].c_str() );
			glDeleteTextures( 1, &m_hTextureID );
			m_hTextureID = 0;
			return;
		}

		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData );
		stbi_image_free( pImageData );
	}

	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
}

TextureGL::~TextureGL()
{
	if ( m_hTextureID != 0 )
		glDeleteTextures( 1, &m_hTextureID );
}

void TextureGL::Bind( unsigned int samplerID, TextureType samplerType )
{
	glActiveTexture( GL_TEXTURE0 + samplerID );
	glBindTexture( g_TextureTypeMap[ samplerType ], m_hTextureID );
}

bool TextureGL::IsValid()
{
	return ( m_hTextureID != 0 );
}