#ifndef RENDERER_OPENGL_HPP
#define RENDERER_OPENGL_HPP

#include "irenderer.hpp"

#include "GL/glew.h"

#include "SDL.h"
#include "SDL_opengl.h"

#include "modelgl.hpp"
#include "skybox.hpp"

#include <vector>

class RendererGL : public IRenderer
{
public:
	RendererGL();
	virtual ~RendererGL();

	virtual bool Init() override;
	virtual void PostInit() override;

	virtual void Shutdown() override;

	virtual void DrawScene() override;
	virtual void Clear() override;
	virtual void Swap() override;

	virtual ITexture *CreateTexture( const string &textureName, bool bCubeMap = false ) override;

	virtual unsigned int GetShaderCount() override;
	virtual IBaseShader *GetShader( unsigned int iShaderIndex ) override;
	virtual IBaseShader *GetShader( const string &shaderName ) override;

	unsigned int AddShader ( IBaseShader *pShader ) override;

	virtual void AddViewPort( IViewPort *pViewPort ) override;
	virtual IViewPort *GetViewPort() override { return m_pViewPorts[ 0 ]; }

private:
	
	SDL_Window *m_pMainWindow;
	SDL_GLContext m_hGLContext;

	std::vector < IBaseShader* > m_pShaders;

	// TODO: Add support for multiple viewports later, for now assume m_pViewPorts[ 0 ] is our main viewport
	std::vector < IViewPort* > m_pViewPorts;

	//MeshGL *m_pMesh;
	ModelGL *m_pModel;
	//MeshGL *m_pFloorMesh;

	Skybox *m_pSkybox;
};

#endif // RENDERER_OPENGL_HPP