#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <map>
#include <vector>

#include "string.hpp"
#include "imaterial.hpp"
#include "renderer/itexture.hpp"

#include "shadersystem/ishader.hpp"
#include "shadersystem/ibaseshader.hpp"

class Material : public IMaterial
{
public:
	Material( const string &materialPath );
	virtual ~Material();

	virtual void Bind();
	virtual bool IsValid();

	virtual ITexture *GetTexture( const string &paramName );
	virtual IBaseShader *GetShader() override { return m_pShader; }

	virtual float GetFloat( const string &paramName );

private:
	std::map < string, ITexture * > m_mapTextures;
	std::map < string, float > m_mapFloats;

	std::vector < MaterialParameter_t > m_vMaterialParameters;

	IBaseShader *m_pShader;
};

#endif // MATERIAL_HPP