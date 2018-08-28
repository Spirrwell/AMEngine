#ifndef ITEXTURE_HPP
#define ITEXTURE_HPP

enum TextureType
{
	TEXTURE_2D,
	TEXTURE_CUBE_MAP
};

class ITexture
{
public:
    virtual ~ITexture() = default;

	virtual void Bind( unsigned int samplerID, TextureType samplerType ) = 0;
	virtual bool IsValid() = 0;
	virtual bool IsCubeMap() = 0;
};

#endif // ITEXTURE_HPP