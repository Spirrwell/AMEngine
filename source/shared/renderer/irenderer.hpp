#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include <stdint.h>
#include "string.hpp"

#include "shadersystem/ishader.hpp"
#include "shadersystem/ibaseshader.hpp"

#include "itexture.hpp"
#include "iviewport.hpp"

class IRenderer
{
public:
    virtual ~IRenderer() = default;

	virtual bool Init() = 0;
	virtual void PostInit() = 0;

	virtual void Shutdown() = 0;

	virtual void DrawScene() = 0;
	virtual void Clear() = 0;
	virtual void Swap() = 0;

	virtual ITexture *CreateTexture( const std::string &textureName, bool bCubeMap = false ) = 0;

	virtual unsigned int GetShaderCount() = 0;
	virtual IBaseShader *GetShader( unsigned int iShaderIndex ) = 0;
	virtual IBaseShader *GetShader( const std::string &shaderName ) = 0;
	virtual unsigned int AddShader ( IBaseShader *pShader ) = 0;

	virtual void AddViewPort( IViewPort *pViewPort ) = 0;
	virtual IViewPort *GetViewPort() = 0;
};

#define RENDERER_INTERFACE "AMRENDERER_V01"

#endif // IRENDERER_HPP