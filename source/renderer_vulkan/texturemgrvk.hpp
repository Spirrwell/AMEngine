#ifndef texturemgrvk_hpp
#define texturemgrvk_hpp

#include "string.hpp"
#include <filesystem>
#include <map>
#include <vector>

class TextureVK;

class TextureMgrVK
{
public:
	static void Shutdown();

	static TextureVK *LoadTexture( const std::filesystem::path &inpath );
	static TextureVK *LoadRGBATextureID( const unsigned char *pPixels, size_t width, size_t height, const string &identifier );
	static TextureVK *LoadSkyTexture( const std::filesystem::path &inpath );

private:
	static std::map< string, TextureVK* > m_pTextureMap;
	static std::vector< TextureVK* > m_pTextures;
};

#endif // texturemgrvk_hpp