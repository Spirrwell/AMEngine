#ifndef TEXTUREGL_HPP
#define TEXTUREGL_HPP

#include <GL/glew.h>

#include <string>
#include <vector>

#include "itexture.hpp"

class TextureGL : public ITexture
{
public:
	TextureGL( const std::string &fileName );
	TextureGL( std::vector < std::string > faces );
	virtual ~TextureGL();

	virtual void Bind( unsigned int samplerID, TextureType samplerType );
	virtual bool IsValid();
	virtual bool IsCubeMap() { return m_bCubeMap; }

private:
	GLuint m_hTextureID;
	bool m_bCubeMap;
};

#endif // TEXTURE_HPP