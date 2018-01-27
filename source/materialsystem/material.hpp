#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <map>
#include <string>
#include <vector>

#include "imaterial.hpp"
#include "renderer/itexture.hpp"

#include "shadersystem/ishader.hpp"
#include "shadersystem/ibaseshader.hpp"

class Material : public IMaterial
{
public:
	Material( const std::string &materialPath );
	virtual ~Material();

	virtual void Bind();
	virtual bool IsValid();

	virtual ITexture *GetTexture( const std::string &paramName );
	virtual IBaseShader *GetShader() override { return m_pShader; }

	virtual float GetFloat( const std::string &paramName );

private:
	std::map < std::string, ITexture * > m_mapTextures;
	std::map < std::string, float > m_mapFloats;

	std::vector < MaterialParameter_t > m_vMaterialParameters;

	IBaseShader *m_pShader;
};

#endif // MATERIAL_HPP