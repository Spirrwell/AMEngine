#include "renderer_vulkan.hpp"
#include "input/iinput.hpp"
#include "interface.hpp"
#include "factory/ifactory.hpp"
#include "materialsystem/imaterialsystem.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

IEngine *g_pEngine = nullptr;
IInput *g_pInput = nullptr;
IMaterialSystem *g_pMaterialSystem = nullptr;

RendererVulkan::RendererVulkan()
{
	m_pMainWindow = nullptr;
}

bool RendererVulkan::Init()
{
	g_pEngine = ( IEngine* )GetFactory()->GetInterface( ENGINE_INTERFACE_VERSION );

	if ( g_pEngine == nullptr )
	{
		printf( "Renderer failed to get engine interface!\n" );
		return false;
	}

	config cfg = g_pEngine->GetConfig();
	Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;

	if ( cfg.windowConfig.fullscreen )
		windowFlags |= SDL_WINDOW_FULLSCREEN;

	m_pMainWindow = SDL_CreateWindow
	(
		"AMEngine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		cfg.windowConfig.resolution_width,
		cfg.windowConfig.resolution_height,
		windowFlags
	);

	if ( m_pMainWindow == nullptr )
	{
		printf( "Renderer: %s\n", SDL_GetError() );
		return false;
	}

	SDL_SetWindowGrab( m_pMainWindow, SDL_TRUE );
	SDL_SetRelativeMouseMode( SDL_TRUE );

	g_pInput = ( IInput* )GetFactory()->GetInterface( INPUT_INTERFACE_VERSION );

	if ( g_pInput == nullptr )
	{
		printf( "Failed to get input interface!\n" );
		return false;
	}

	/*if ( !GetFactory()->LoadModule( "bin/shadervulkan" + string( DLL_EXTENSION ) ) )
    {
        printf( "Renderer failed to get shadervulkan module\n" );
		return false;
    }*/

	/*g_pShaderManager = ( IShaderManager* )GetFactory()->GetInterface( SHADERMANAGER_INTERFACE );

	if ( !g_pShaderManager || !g_pShaderManager->Init() )
		return false;*/

	if ( !GetFactory()->LoadModule( "bin/materialsystem" + string( DLL_EXTENSION ) ) )
    {
        printf( "Renderer failed to get materialsystem module\n" );
		return false;
    }

	g_pMaterialSystem = ( IMaterialSystem* )GetFactory()->GetInterface( MATERIALSYSTEM_INTERFACE_VERSION );

	if ( !g_pMaterialSystem || !g_pMaterialSystem->Init() )
		return false;

	if ( !m_vkApp.initVulkan() )
	{
		m_vkApp.printError();
		return false;
	}

	/*uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

	stprintf( "Extension Count: %u\n", extensionCount );*/

	return true;
}

void RendererVulkan::PostInit()
{

}

void RendererVulkan::Shutdown()
{
	m_vkApp.cleanup();

	if ( m_pMainWindow != nullptr )
	{
		SDL_DestroyWindow( m_pMainWindow );
		m_pMainWindow = nullptr;
	}
}

void RendererVulkan::DrawScene()
{
	static uint32_t imageIndex = 0;
	vkAcquireNextImageKHR( m_vkApp.vulkan().device, m_vkApp.vulkan().swapChain, std::numeric_limits< uint64_t >::max(), m_vkApp.vulkan().imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_vkApp.vulkan().imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkApp.vulkan().commandBuffers[ imageIndex ];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_vkApp.vulkan().renderFinishedSemaphore;

	if ( vkQueueSubmit( m_vkApp.vulkan().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
	{
		stprintf( "[Vulkan]Queue submit failed!\n" );
		return;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_vkApp.vulkan().renderFinishedSemaphore;

	VkSwapchainKHR swapChains[] = { m_vkApp.vulkan().swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR( m_vkApp.vulkan().presentQueue, &presentInfo );
	vkQueueWaitIdle( m_vkApp.vulkan().presentQueue );
}

static DLLInterface< IRenderer, RendererVulkan > s_Renderer( RENDERER_INTERFACE );
RendererVulkan &GetVkRenderer_Internal() { return *s_Renderer.GetInternal(); }