#ifndef RENDERER_VULKAN_HPP
#define RENDERER_VULKAN_HPP

#include "irenderer.hpp"

#include "SDL.h"
#include "SDL_vulkan.h"

#include "vulkan/vulkan.hpp"
#include "vulkan_helpers.hpp"

class RendererVulkan : public IRenderer
{
public:
	RendererVulkan();
	virtual ~RendererVulkan() = default;

	bool Init() override;
	void PostInit() override;

	void Shutdown() override;

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

	SDL_Window *GetWindow() { return m_pMainWindow; }

private:
	SDL_Window *m_pMainWindow = nullptr;
	vkApp::VulkanApp m_vkApp;

	RendererVulkan( const RendererVulkan& ) = delete;
};

extern RendererVulkan &GetVkRenderer_Internal();

#endif // RENDERER_VULKAN_HPP