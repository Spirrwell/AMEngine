#ifndef TEXTUREGL_HPP
#define TEXTUREGL_HPP

#include <GL/glew.h>

#include <vector>

#include "string.hpp"
#include "itexture.hpp"

class TextureGL : public ITexture
{
public:
	TextureGL( const string &fileName );
	TextureGL( std::vector < string > faces );
	virtual ~TextureGL();

	virtual void Bind( unsigned int samplerID, TextureType samplerType );
	virtual bool IsValid();
	virtual bool IsCubeMap() { return m_bCubeMap; }

private:
	GLuint m_hTextureID;
	bool m_bCubeMap;
};

#endif // TEXTURE_HPP