#ifndef RENDERER_EMPTY_HPP
#define RENDERER_EMPTY_HPP

#include "irenderer.hpp"

class RendererEmpty : public IRenderer
{
public:
	bool Init() override { return true; }
	void PostInit() override {}

	void Shutdown() override {}

	void DrawScene() override {}
	void Clear() override {}
	void Swap() override {}

	ITexture *CreateTexture( const string &textureName, bool bCubemap = false ) override { return nullptr; }

	unsigned int GetShaderCount() override { return 0; }
	IBaseShader *GetShader( unsigned int iShaderIndex ) override { return nullptr; }
	IBaseShader *GetShader( const string &shaderName ) override { return nullptr; }

	unsigned int AddShader( IBaseShader *pShader ) override { return 0; }

	void AddViewPort( IViewPort *pViewPort ) override {}
	IViewPort *GetViewPort() override { return nullptr; }
};

#endif // RENDERER_EMPTY_HPP