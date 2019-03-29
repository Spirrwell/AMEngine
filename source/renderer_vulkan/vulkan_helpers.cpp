#include "vulkan_helpers.hpp"
#include "renderer_vulkan.hpp"
#include "engine/config.hpp"
#include "vertex.hpp"
#include "shadervk.hpp"
#include "materialvk.hpp"
#include "meshvk.hpp"
#include "modelvk.hpp"

#include "SDL_vulkan.h"
#include "mathdefs.hpp"
#include "amdl.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <array>
#include <map>
#include <set>
#include <chrono>
#include "testshader.hpp"
#include "camera.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

Camera g_vkcam( Vector3f( 0.0f, -5.0f, 0.0f ) );

static std::vector< Vector3f > skyboxVertices = {
	// positions
	{ -1.0f,  1.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f },
	{ 1.0f, -1.0f, -1.0f },
	{ 1.0f, -1.0f, -1.0f },
	{ 1.0f,  1.0f, -1.0f },
	{ -1.0f,  1.0f, -1.0f },

	{ -1.0f, -1.0f,  1.0f },
	{ -1.0f, -1.0f, -1.0f },
	{ -1.0f,  1.0f, -1.0f },
	{ -1.0f,  1.0f, -1.0f },
	{ -1.0f,  1.0f,  1.0f },
	{ -1.0f, -1.0f,  1.0f },

	{ 1.0f, -1.0f, -1.0f },
	{ 1.0f, -1.0f,  1.0f },
	{ 1.0f,  1.0f,  1.0f },
	{ 1.0f,  1.0f,  1.0f },
	{ 1.0f,  1.0f, -1.0f },
	{ 1.0f, -1.0f, -1.0f },

	{ -1.0f, -1.0f,  1.0f },
	{ -1.0f,  1.0f,  1.0f },
	{ 1.0f,  1.0f,  1.0f },
	{ 1.0f,  1.0f,  1.0f },
	{ 1.0f, -1.0f,  1.0f },
	{ -1.0f, -1.0f,  1.0f },

	{ -1.0f,  1.0f, -1.0f },
	{ 1.0f,  1.0f, -1.0f },
	{ 1.0f,  1.0f,  1.0f },
	{ 1.0f,  1.0f,  1.0f },
	{ -1.0f,  1.0f,  1.0f },
	{ -1.0f,  1.0f, -1.0f },

	{ -1.0f, -1.0f, -1.0f },
	{ -1.0f, -1.0f,  1.0f },
	{ 1.0f, -1.0f, -1.0f },
	{ 1.0f, -1.0f, -1.0f },
	{ -1.0f, -1.0f,  1.0f },
	{ 1.0f, -1.0f,  1.0f }
};

namespace vkApp
{
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 mvp;
	};

	static vk::VertexInputBindingDescription getVertexBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof( Vertex );
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		return bindingDescription;
	}

	static std::array< vk::VertexInputAttributeDescription, 4 > getVertexAttributeDescriptions()
	{
		std::array< vk::VertexInputAttributeDescription, 4 > attributeDescriptions = {};

		attributeDescriptions[ 0 ].binding = 0;
		attributeDescriptions[ 0 ].location = 0;
		attributeDescriptions[ 0 ].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

		attributeDescriptions[ 1 ].binding = 0;
		attributeDescriptions[ 1 ].location = 1;
		attributeDescriptions[ 1 ].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

		attributeDescriptions[ 2 ].binding = 0;
		attributeDescriptions[ 2 ].location = 2;
		attributeDescriptions[ 2 ].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );

		attributeDescriptions[ 3 ].binding = 0;
		attributeDescriptions[ 3 ].location = 3;
		attributeDescriptions[ 3 ].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[ 3 ].offset = offsetof( Vertex, normal );

		return attributeDescriptions;
	}

	/*std::vector< Vertex > vertices =
	{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
	};*/

	std::vector< Vertex > vertices =
	{
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },

		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
	};

	std::vector< uint32_t > indexBuffer =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	static std::vector< std::byte > readFile( const string &fileName )
	{
		std::ifstream file( fileName, std::ios::ate | std::ios::binary );

		if ( !file.is_open() )
			return {};

		size_t fileSize = static_cast< size_t >( file.tellg() );
		std::vector< std::byte > buffer( fileSize );

		file.seekg( 0 );
		file.read( ( char* )buffer.data(), fileSize );

		file.close();
		
		return buffer;
	}

	VulkanApp::VulkanApp()
	{
	}

	VulkanApp::~VulkanApp()
	{
		cleanup();
	}

	bool VulkanApp::initVulkan()
	{
		loadExtensionsFromSDL();
#if VULKAN_VALIDATION_LAYERS
		checkValidationLayerSupport();
#endif
		createInstance();
#if VULKAN_VALIDATION_LAYERS
		setupDebugCallback();
#endif
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		//createDescriptorSetLayout();
		//createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createFramebuffers();

		vulkan().texture.Load( GAME_DIR "textures/chalet.jpg" );

		AMDL::ModelData modelData;
		//AMDL::ReadAMDLFile( string( GAME_DIR ) + "models/cube.amdl", modelData );
		/*AMDL::ReadAMDLFile( string( GAME_DIR ) + "models/chalet_mat.amdl", modelData );

		if ( modelData.meshes.size() > 0 )
		{
			vertices.clear();
			indexBuffer.clear();

			vertices = modelData.meshes[ 0 ].vertices;
			indexBuffer = modelData.meshes[ 0 ].indices;
		}*/

		//createVertexBuffer();
		//createIndexBuffer();
		//createUniformBuffer();
		//createDescriptorPool();
		//createDescriptorSets();

		for ( const auto &pShader : m_pShaders )
			pShader->Init();

		m_pTestModel = new ModelVK;
		m_pTestModel->SetModelMatrix( glm::translate( Vector3f( 0.0f, 2.0f, 0.0f ) ) );
		m_pTestModel->LoadModel( string( GAME_DIR ) + "models/axis.amdl" );

		m_pTestModel2 = new ModelVK;
		m_pTestModel2->SetModelMatrix( glm::translate( Vector3f( 0.0f, 2.0f, 0.0f ) ) );
		m_pTestModel2->LoadModel( string( GAME_DIR ) + "models/chalet_mat.amdl" );

		std::vector< Vertex > skyVerts;
		skyVerts.resize( skyboxVertices.size() );
		for ( size_t i = 0; i < skyboxVertices.size(); ++i )
			skyVerts[ i ].pos = skyboxVertices[ i ];

		m_pSkyboxMaterial = new MaterialVK( string( GAME_DIR ) + "materials/skybox/default_sky.amat" );
		m_pSkyboxTest = new MeshVK( skyVerts, {}, m_pSkyboxMaterial );

		allocateCommandBuffers();
		recordCommandBuffers();
		createSyncObjects();

		//m_pTestMaterial = new MaterialVK( string( GAME_DIR ) + "materials/cube/BasicMaterial.amat" );
		//m_pTestMaterial = new MaterialVK( string( GAME_DIR ) + "materials/chalet/chalet.amat" );
		//m_pTestMesh = new MeshVK( vertices, indexBuffer, m_pTestMaterial );

		return true;
	}

	void VulkanApp::cleanup()
	{
		if ( vulkan().device )
			vulkan().device.waitIdle();

		/*if ( m_pTestMesh )
		{
			delete m_pTestMesh;
			m_pTestMesh = nullptr;
		}

		delete m_pTestMaterial;
		m_pTestMaterial = nullptr;*/

		if ( m_pSkyboxMaterial )
		{
			delete m_pSkyboxMaterial;
			m_pSkyboxMaterial = nullptr;
		}

		if ( m_pSkyboxTest )
		{
			delete m_pSkyboxTest;
			m_pSkyboxTest = nullptr;
		}

		delete m_pTestModel;
		m_pTestModel = nullptr;

		delete m_pTestModel2;
		m_pTestModel2 = nullptr;

		for ( const auto &pShader : m_pShaders )
			pShader->Shutdown();

		cleanupSwapChain();

		vulkan().texture.Shutdown();

		if ( vulkan().descriptorPool )
		{
			vulkan().device.destroyDescriptorPool( vulkan().descriptorPool, nullptr );
			vulkan().descriptorPool = nullptr;
		}

		if ( vulkan().descriptorSetLayout )
		{
			vulkan().device.destroyDescriptorSetLayout( vulkan().descriptorSetLayout, nullptr );
			vulkan().descriptorSetLayout = nullptr;
		}

		vulkan().descriptorSets.clear();

		for ( size_t i = 0; i < vulkan().uniformBuffers.size(); ++i )
		{
			vulkan().device.destroyBuffer( vulkan().uniformBuffers[ i ], nullptr );
			vulkan().device.freeMemory( vulkan().uniformBuffersMemory[ i ], nullptr );
		}

		vulkan().uniformBuffers.clear();
		vulkan().uniformBuffersMemory.clear();

		if ( vulkan().indexBuffer )
		{
			vulkan().device.destroyBuffer( vulkan().indexBuffer, nullptr );
			vulkan().indexBuffer = nullptr;
		}

		if ( vulkan().indexBufferMemory )
		{
			vulkan().device.freeMemory( vulkan().indexBufferMemory, nullptr );
			vulkan().indexBufferMemory = nullptr;
		}

		if ( vulkan().vertexBuffer )
		{
			vulkan().device.destroyBuffer( vulkan().vertexBuffer, nullptr );
			vulkan().vertexBuffer = nullptr;
		}

		if ( vulkan().vertexBufferMemory )
		{
			vulkan().device.freeMemory( vulkan().vertexBufferMemory, nullptr );
			vulkan().vertexBufferMemory = nullptr;
		}

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
		{
			if ( vulkan().renderFinishedSemaphores.size() > 0 )
			{
				if ( vulkan().renderFinishedSemaphores[ i ] )
					vulkan().device.destroySemaphore( vulkan().renderFinishedSemaphores[ i ], nullptr );
			}

			if ( vulkan().imageAvailableSemaphores.size() > 0 )
			{
				if ( vulkan().imageAvailableSemaphores[ i ] )
					vulkan().device.destroySemaphore( vulkan().imageAvailableSemaphores[ i ], nullptr );
			}

			if ( vulkan().inFlightFences.size() > 0 )
			{
				if ( vulkan().inFlightFences[ i ] )
					vulkan().device.destroyFence( vulkan().inFlightFences[ i ], nullptr );
			}
		}

		vulkan().renderFinishedSemaphores.clear();
		vulkan().imageAvailableSemaphores.clear();
		vulkan().inFlightFences.clear();

		if ( vulkan().commandPool )
		{
			vulkan().device.destroyCommandPool( vulkan().commandPool, nullptr );
			vulkan().commandPool = nullptr;
		}

		if ( vulkan().device )
		{
			vulkan().device.destroy( nullptr );
			vulkan().device = nullptr;

			vulkan().graphicsQueue = nullptr; // This is cleaned up when the device is destroyed
			vulkan().presentQueue = nullptr; // This is cleaned up when the device is destroyed
		}

#if VULKAN_VALIDATION_LAYERS
		if ( vulkan().debugCallback )
		{
			DestroyDebugUtilsMessengerEXT( vulkan().instance, vulkan().debugCallback, nullptr );
			vulkan().debugCallback = nullptr;
		}
#endif

		if ( vulkan().surface )
		{
			vulkan().instance.destroySurfaceKHR( vulkan().surface );
			vulkan().surface = nullptr;
		}

		if ( vulkan().instance )
		{
			vkDestroyInstance( vulkan().instance, nullptr );
			vulkan().instance = nullptr;
			vulkan().physicalDevice = nullptr; // This is cleaned up when the instance is destroyed
		}

		if ( m_ppszReqExtensionNames )
		{
			delete[] m_ppszReqExtensionNames;
			m_ppszReqExtensionNames = nullptr;
		}
	}

	void VulkanApp::cleanupSwapChain()
	{
		if ( vulkan().depthImageView )
		{
			vulkan().device.destroyImageView( vulkan().depthImageView, nullptr );
			vulkan().depthImageView = nullptr;
		}

		if ( vulkan().depthImage )
		{
			vulkan().device.destroyImage( vulkan().depthImage, nullptr );
			vulkan().depthImage = nullptr;
		}

		if ( vulkan().depthImageMemory )
		{
			vulkan().device.freeMemory( vulkan().depthImageMemory, nullptr );
			vulkan().depthImageMemory = nullptr;
		}

		for ( auto &framebuffer : vulkan().swapChainFramebuffers )
		{
			if ( framebuffer )
				vulkan().device.destroyFramebuffer( framebuffer, nullptr );
		}

		vulkan().swapChainFramebuffers.clear();

		if ( vulkan().commandBuffers.size() > 0 )
			vulkan().device.freeCommandBuffers( vulkan().commandPool, static_cast< uint32_t >( vulkan().commandBuffers.size() ), vulkan().commandBuffers.data() );

		vulkan().commandBuffers.clear();

		if ( vulkan().graphicsPipeline )
		{
			vulkan().device.destroyPipeline( vulkan().graphicsPipeline, nullptr );
			vulkan().graphicsPipeline = nullptr;
		}

		if ( vulkan().pipelineLayout )
		{
			vulkan().device.destroyPipelineLayout( vulkan().pipelineLayout, nullptr );
			vulkan().pipelineLayout = nullptr;
		}

		if ( vulkan().renderPass )
		{
			vulkan().device.destroyRenderPass( vulkan().renderPass, nullptr );
			vulkan().renderPass = nullptr;
		}

		for ( auto &imageView : vulkan().swapChainImageViews )
		{
			if ( imageView )
				vulkan().device.destroyImageView( imageView, nullptr );
		}

		vulkan().swapChainImageViews.clear();
		vulkan().swapChainImages.clear();

		if ( vulkan().swapChain )
		{
			vulkan().device.destroySwapchainKHR( vulkan().swapChain, nullptr );
			vulkan().swapChain = nullptr;
		}
	}

	void VulkanApp::drawFrame()
	{
		static uint32_t imageIndex = 0;
		static size_t currentFrame = 0;

		if ( vulkan().bMinimized )
			return;

		vulkan().device.waitForFences( 1, &vulkan().inFlightFences[ currentFrame ], VK_TRUE, std::numeric_limits< uint64_t >::max() );
		vk::Result result = vulkan().device.acquireNextImageKHR( vulkan().swapChain, std::numeric_limits< uint64_t >::max(), vulkan().imageAvailableSemaphores[ currentFrame ], nullptr, &imageIndex );
		//updateUniformBuffer( imageIndex );

		if ( result == vk::Result::eErrorOutOfDateKHR )
		{
			recreateSwapChain();
			return;
		}
		else if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
		{
			stprintf( "[Vulkan]Failed to acquire swap chain image.\n" );
			return;
		}

		g_vkcam.Update();
		g_vkcam.UpdateView();

		recordCommandBuffer( imageIndex );

		vk::Semaphore waitSemaphores[] = { vulkan().imageAvailableSemaphores[ currentFrame ] };

		vk::SubmitInfo submitInfo;

		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vulkan().commandBuffers[ imageIndex ];

		vk::Semaphore signalSemaphores[] = { vulkan().renderFinishedSemaphores[ currentFrame ] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vulkan().device.resetFences( 1, &vulkan().inFlightFences[ currentFrame ] );

		if ( vulkan().graphicsQueue.submit( 1, &submitInfo, vulkan().inFlightFences[ currentFrame ] ) != vk::Result::eSuccess )
		{
			stprintf( "[Vulkan]Queue submit failed!\n" );
			return;
		}

		vk::PresentInfoKHR presentInfo;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		vk::SwapchainKHR swapChains[] = { vulkan().swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		result = vulkan().presentQueue.presentKHR( presentInfo );

		if ( result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || vulkan().bFrameBufferResized )
		{
			vulkan().bFrameBufferResized = false;
			recreateSwapChain();
			return;
		}
		else if ( result != vk::Result::eSuccess )
		{
			stprintf( "[Vulkan]Failed to present swap chain image.\n" );
			return;
		}
		
		currentFrame = ( currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanApp::updateUniformBuffer( const uint32_t &currentImage )
	{
		/*static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration< float, std::chrono::seconds::period >( currentTime - startTime ).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate( Matrix4f( 1.0f ), time * glm::radians( 90.0f ), Vector3f( 0.0f, 0.0f, 1.0f ) );
		ubo.view = glm::lookAt( Vector3f( 2.0f, 2.0f, 2.0f ), Vector3f( 0.0f, 0.0f, 0.0f ), Vector3f( 0.0f, 0.0f, 1.0f ) );
		ubo.proj = glm::perspective( glm::radians( 45.0f ), ( float )vulkan().swapChainExtent.width / ( float ) vulkan().swapChainExtent.height, 0.1f, 10.0f );
		//ubo.proj[ 1 ][ 1 ] *= -1;
		ubo.mvp = ubo.proj * ubo.view * ubo.model;

		g_tspc.mvp = ubo.mvp;

		void *pData = nullptr;
		vkMapMemory( vulkan().device, vulkan().uniformBuffersMemory[ currentImage ], 0, sizeof( ubo ), 0, &pData );
			std::memcpy( pData, &ubo, sizeof( ubo ) );
		vkUnmapMemory( vulkan().device, vulkan().uniformBuffersMemory[ currentImage ] );*/
	}

#if VULKAN_VALIDATION_LAYERS
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData )
	{
		stprintf( "[Vulkan]Validation Layer: %s\n", pCallbackData->pMessage );
		return VK_FALSE;
	}

	vk::Result VulkanApp::CreateDebugUtilsMessengerEXT( vk::Instance instance,
		const vk::DebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const vk::AllocationCallbacks *pAllocator,
		vk::DebugUtilsMessengerEXT *pCallback )
	{
		auto func = ( PFN_vkCreateDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		return ( func ) ? vk::Result( func( instance, reinterpret_cast< const VkDebugUtilsMessengerCreateInfoEXT* >( pCreateInfo ), reinterpret_cast< const VkAllocationCallbacks* >( pAllocator ), reinterpret_cast< VkDebugUtilsMessengerEXT* >( pCallback ) ) ) : vk::Result::eErrorExtensionNotPresent;
	}

	void VulkanApp::DestroyDebugUtilsMessengerEXT( vk::Instance instance, vk::DebugUtilsMessengerEXT callback, const vk::AllocationCallbacks *pAllocator )
	{
		auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if ( func != nullptr )
			func( instance, callback, reinterpret_cast< const VkAllocationCallbacks* >( pAllocator ) );
	}

	void VulkanApp::checkValidationLayerSupport()
	{
		uint32_t layerCount = 0;
		vk::enumerateInstanceLayerProperties( &layerCount, nullptr );

		std::vector< vk::LayerProperties > availableLayers( static_cast< size_t >( layerCount ) );
		vk::enumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

		for ( auto &layerName : vulkan().validationLayers )
		{
			bool bLayerFound = false;

			for ( const auto &layerProperties : availableLayers )
			{
				if ( strcmp( layerName, layerProperties.layerName ) == 0 )
				{
					bLayerFound = true;
					break;
				}
			}

			if ( !bLayerFound )
				throw std::runtime_error( "[Vulkan]Failed to get validation layers." );
		}
	}
#endif

	void VulkanApp::ProcessEvent( const SDL_Event &event )
	{
		if ( event.type == SDL_WINDOWEVENT )
		{
			config &cfg = g_pEngine->GetConfig();

			switch( event.window.event )
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				vulkan().bFrameBufferResized = true;

				cfg.windowConfig.resolution_width = event.window.data1;
				cfg.windowConfig.resolution_height = event.window.data2;
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				vulkan().bMinimized = true;

				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
			case SDL_WINDOWEVENT_RESTORED:
				vulkan().bMinimized = false;
				break;
			default:
				break;
			}
		}
	}

	void VulkanApp::loadExtensionsFromSDL()
	{
		unsigned int extensionCount = 0;
		SDL_Vulkan_GetInstanceExtensions( GetVkRenderer_Internal().GetWindow(), &extensionCount, nullptr );

		if ( extensionCount == 0 )
			throw std::runtime_error( "[Vulkan]Failed to load required extensions from SDL." );

		m_ppszReqExtensionNames = new const char *[ extensionCount ];
		SDL_Vulkan_GetInstanceExtensions( GetVkRenderer_Internal().GetWindow(), &extensionCount, m_ppszReqExtensionNames );

		m_iReqExtCount = static_cast< uint32_t >( extensionCount );
	}

	void VulkanApp::createInstance()
	{
		vk::ApplicationInfo appInfo;
		appInfo.pApplicationName = "AMEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.pEngineName = "AMEngine";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_1;

		vk::InstanceCreateInfo createInfo;
		createInfo.pApplicationInfo = &appInfo;

		auto reqExtensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast< uint32_t >( reqExtensions.size() );
		createInfo.ppEnabledExtensionNames = reqExtensions.data();

#if VULKAN_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast< uint32_t >( vulkan().validationLayers.size() );
		createInfo.ppEnabledLayerNames = vulkan().validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		auto[ result, instance ] = vk::createInstance( createInfo, nullptr );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create instance." );

		vulkan().instance = std::move( instance );

		// Query available extensions
		uint32_t extensionCount = 0;
		vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

		vulkan().extensions.resize( static_cast< size_t >( extensionCount ) );
		vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, vulkan().extensions.data() );

		stprintf( "[Vulkan]Available Extensions:\n" );

		for ( const auto &extension : vulkan().extensions )
			stprintf( "\t%s\n", extension.extensionName );

		stprintf( "\n" );
	}

#if VULKAN_VALIDATION_LAYERS
	void VulkanApp::setupDebugCallback()
	{
		vk::DebugUtilsMessengerCreateInfoEXT createInfo;
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		if ( CreateDebugUtilsMessengerEXT( vulkan().instance, &createInfo, nullptr, &vulkan().debugCallback ) != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to setup debug callback." );
	}
#endif

	void VulkanApp::createSurface()
	{
		if ( SDL_Vulkan_CreateSurface( GetVkRenderer_Internal().GetWindow(), vulkan().instance, ( VkSurfaceKHR* )&vulkan().surface ) != SDL_TRUE )
			throw std::runtime_error( "[Vulkan]Failed to create window surface." );
	}

	void VulkanApp::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vulkan().instance.enumeratePhysicalDevices( &deviceCount, nullptr );

		if ( deviceCount == 0 )
			throw std::runtime_error( "[Vulkan]No physical device found." );

		std::vector< vk::PhysicalDevice > devices( static_cast< size_t >( deviceCount ) );
		vulkan().instance.enumeratePhysicalDevices( &deviceCount, devices.data() );

		auto rateSuitability = []( vk::PhysicalDevice &device )
		{
			uint32_t iScore = 1;

			vk::PhysicalDeviceProperties deviceProperties;
			device.getProperties( &deviceProperties );

			vk::PhysicalDeviceFeatures deviceFeatures;
			device.getFeatures( &deviceFeatures );

			if ( deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu )
				iScore += 1000;

			iScore += deviceProperties.limits.maxImageDimension2D;

			if ( !deviceFeatures.geometryShader )
				return static_cast< uint32_t >( 0 );

			if ( !deviceFeatures.textureCompressionBC )
				return static_cast< uint32_t >( 0 );

			return iScore;
		};

		// Ordered map that automatically sorts device candidates by increasing score
		std::multimap< uint32_t, vk::PhysicalDevice > candidates;

		for ( vk::PhysicalDevice device : devices )
		{
			if ( !isDeviceSuitable( device ) )
				continue;

			uint32_t score = rateSuitability( device );
			candidates.insert( std::make_pair( score, device ) );
		}

		if ( !candidates.empty() && candidates.rbegin()->first > 0 )
			vulkan().physicalDevice = candidates.rbegin()->second;

		if ( !vulkan().physicalDevice )
			throw std::runtime_error( "[Vulkan]No physical device found." );
	}

	bool VulkanApp::isDeviceSuitable( vk::PhysicalDevice &device )
	{
		auto checkDeviceExtensionSupport = [ &, this ]()
		{
			uint32_t extensionCount = 0;
			device.enumerateDeviceExtensionProperties( nullptr, &extensionCount, nullptr );

			std::vector< vk::ExtensionProperties > availableExtensions( static_cast< size_t >( extensionCount ) );
			device.enumerateDeviceExtensionProperties( nullptr, &extensionCount, availableExtensions.data() );

			std::set< string > requiredExtensions( vulkan().deviceExtensions.begin(), vulkan().deviceExtensions.end() );

			for ( const auto &extension : availableExtensions )
				requiredExtensions.erase( extension.extensionName );

			return requiredExtensions.empty();
		};

		auto checkSwapChainSupport = [ & ]( std::shared_ptr< SwapChainSupportDetails > swapChainSupport )
		{
			return ( !swapChainSupport->formats.empty() && !swapChainSupport->presentModes.empty() );
		};

		auto checkDeviceFeatures = [ & ]()
		{
			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures( device, &supportedFeatures );

			return static_cast< bool >( supportedFeatures.samplerAnisotropy );
		};


		return ( findQueueFamilies( device ).isComplete() &&
			checkDeviceExtensionSupport() &&
			checkSwapChainSupport( querySwapChainSupport( device ) ) &&
			checkDeviceFeatures() );
	}

	void VulkanApp::createLogicalDevice()
	{
		// TODO: Cache this
		auto indices = findQueueFamilies( vulkan().physicalDevice );

		std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos;
		std::set< uint32_t > uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for ( auto queueFamily : uniqueQueueFamilies )
		{
			vk::DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back( queueCreateInfo );
		}

		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.textureCompressionBC = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		vk::DeviceCreateInfo createInfo = {};
		createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size() );
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast< uint32_t >( vulkan().deviceExtensions.size() );
		createInfo.ppEnabledExtensionNames = vulkan().deviceExtensions.data();

#if VULKAN_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast< uint32_t >( vulkan().validationLayers.size() );
		createInfo.ppEnabledLayerNames = vulkan().validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		auto[ result, logicalDevice ] = vulkan().physicalDevice.createDevice( createInfo, nullptr );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create logical device." );

		vulkan().device = std::move( logicalDevice );
			
		vulkan().device.getQueue( indices.graphicsFamily, 0, &vulkan().graphicsQueue );
		vulkan().device.getQueue( indices.presentFamily, 0, &vulkan().presentQueue );
	}

	void VulkanApp::createSwapChain()
	{
		auto swapChainSupport = querySwapChainSupport( vulkan().physicalDevice );

		vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport->formats );
		vk::PresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport->presentModes );
		vk::Extent2D extent = chooseSwapExtent( swapChainSupport->capabilities );

		uint32_t imageCount = swapChainSupport->capabilities.maxImageCount > 0 ?
			std::min( swapChainSupport->capabilities.minImageCount + 1, swapChainSupport->capabilities.maxImageCount ) :
			swapChainSupport->capabilities.minImageCount + 1;

		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.surface = vulkan().surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		auto indices = findQueueFamilies( vulkan().physicalDevice );
		uint32_t queueFamilyIndices[] =
		{
			indices.graphicsFamily,
			indices.presentFamily
		};

		if ( indices.graphicsFamily != indices.presentFamily )
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport->capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; // TODO: Revisit this for window blending
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;

		if ( auto[ result, swapChain ] = vulkan().device.createSwapchainKHR( createInfo, nullptr ); result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create swap chain." );
		else
			vulkan().swapChain = std::move( swapChain );		

		vulkan().device.getSwapchainImagesKHR( vulkan().swapChain, &imageCount, nullptr );
		vulkan().swapChainImages.resize( imageCount, nullptr );
		vulkan().device.getSwapchainImagesKHR( vulkan().swapChain, &imageCount, vulkan().swapChainImages.data() );

		vulkan().swapChainImageFormat = surfaceFormat.format;
		vulkan().swapChainExtent = extent;
	}

	void VulkanApp::createImageViews()
	{
		vulkan().swapChainImageViews.resize( vulkan().swapChainImages.size(), nullptr );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); i++ )
			vulkan().swapChainImageViews[ i ] = createImageView( vulkan().swapChainImages[ i ], vulkan().swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1, vk::ImageViewType::e2D );
	}

	void VulkanApp::createRenderPass()
	{
		vk::AttachmentDescription colorAttachment;
		colorAttachment.format = vulkan().swapChainImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentDescription depthAttachment ;
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef;
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		vk::SubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlags();
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		std::array< vk::AttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		auto[ result, renderPass ] = vulkan().device.createRenderPass( renderPassInfo, nullptr );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create render pass." );

		vulkan().renderPass = std::move( renderPass );
	}

	void VulkanApp::createDescriptorSetLayout()
	{
		vk::DescriptorSetLayoutBinding uboLayoutBinding;
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		vk::DescriptorSetLayoutBinding samplerLayoutBinding;
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

		std::array< vk::DescriptorSetLayoutBinding, 2 > bindings = { uboLayoutBinding, samplerLayoutBinding };
		vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size() );
		layoutInfo.pBindings = bindings.data();

		auto[ result, descriptorSetLayout ] = vulkan().device.createDescriptorSetLayout( layoutInfo, nullptr );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create descriptor set layout!\n" );

		vulkan().descriptorSetLayout = std::move( descriptorSetLayout );
	}

	void VulkanApp::createGraphicsPipeline()
	{
		enum shaderType : size_t
		{
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			SHADER_COUNT
		};

		std::vector< std::byte > shaderCode[ SHADER_COUNT ] =
		{
			readFile( string( GAME_DIR ) + string( "shaders/testShader.vert.spv" ) ),
			readFile( string( GAME_DIR ) + string( "shaders/testShader.frag.spv" ) )
		};

		for ( const auto &shader : shaderCode )
		{
			if ( shader.empty() )
				throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
		}

		vk::ShaderModule shaderModules[ SHADER_COUNT ];

		for ( size_t i = 0; i < SHADER_COUNT; i++ )
		{
			shaderModules[ i ] = createShaderModule( shaderCode[ i ] );

			if ( !shaderModules[ i ] )
				throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
		}

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
		vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertShaderStageInfo.module = shaderModules[ VERTEX_SHADER ];
		vertShaderStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
		fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragShaderStageInfo.module = shaderModules[ FRAGMENT_SHADER ];
		fragShaderStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		auto bindingDescription = getVertexBindingDescription();
		auto attributeDescriptions = getVertexAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size() );
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = ( float )vulkan().swapChainExtent.width;
		viewport.height = ( float )vulkan().swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = vulkan().swapChainExtent;

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterizer;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack; // TODO: Revisit this
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise; // TODO: Revisit this
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optioanl
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		vk::PipelineMultisampleStateCreateInfo multisampling;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		/*colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		/*Alpha Blending*/
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo colorBlending;
		colorBlending.logicOpEnable = VK_TRUE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
		colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
		colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
		colorBlending.blendConstants[ 3 ] = 0.0f; // Optional

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &vulkan().descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		auto destroyShaderModules = [ & ]()
		{
			for ( auto &shaderModule : shaderModules )
				vkDestroyShaderModule( vulkan().device, shaderModule, nullptr );
		};

		if ( auto[ result, pipelineLayout ] = vulkan().device.createPipelineLayout( pipelineLayoutInfo, nullptr ); result == vk::Result::eSuccess )
			vulkan().pipelineLayout = std::move( pipelineLayout );
		else
		{
			destroyShaderModules();
			throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
		}

		vk::PipelineDepthStencilStateCreateInfo depthStencil;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = vk::CompareOp::eLess;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = vulkan().pipelineLayout;
		pipelineInfo.renderPass = vulkan().renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		auto[ result, graphicsPipeline ] = vulkan().device.createGraphicsPipeline( nullptr, pipelineInfo, nullptr );

		if ( result != vk::Result::eSuccess )
		{
			destroyShaderModules();
			throw std::runtime_error( "[Vulkan]Failed to create graphics pipeline." );
		}

		vulkan().graphicsPipeline = std::move( graphicsPipeline );

		destroyShaderModules();
	}

	void VulkanApp::createFramebuffers()
	{
		vulkan().swapChainFramebuffers.resize( vulkan().swapChainImageViews.size(), nullptr );

		for ( size_t i = 0; i < vulkan().swapChainFramebuffers.size(); ++i )
		{
			std::array< vk::ImageView, 2 > attachments = { vulkan().swapChainImageViews[ i ], vulkan().depthImageView };

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.renderPass = vulkan().renderPass;
			framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = vulkan().swapChainExtent.width;
			framebufferInfo.height = vulkan().swapChainExtent.height;
			framebufferInfo.layers = 1;

			auto[ result, framebuffer ] = vulkan().device.createFramebuffer( framebufferInfo, nullptr );

			if ( result != vk::Result::eSuccess )
				throw std::runtime_error( "[Vulkan]Failed to create framebuffers." );

			vulkan().swapChainFramebuffers[ i ] = std::move( framebuffer );
		}
	}

	void VulkanApp::createCommandPool()
	{
		auto queueFamilyIndices = findQueueFamilies( vulkan().physicalDevice );

		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer; // Optional

		auto[ result, commandPool ] = vulkan().device.createCommandPool( poolInfo, nullptr );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create command pool." );

		vulkan().commandPool = std::move( commandPool );
	}

	void VulkanApp::createDepthResources()
	{
		vk::Format depthFormat = findDepthFormat();
		createImage( vulkan().swapChainExtent.width, vulkan().swapChainExtent.height, 1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkan().depthImage, vulkan().depthImageMemory );
		vulkan().depthImageView = createImageView( vulkan().depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1, vk::ImageViewType::e2D );
		transitionImageLayout( vulkan().depthImage, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1 );
	}

	void VulkanApp::createVertexBuffer()
	{
		vk::DeviceSize bufferSize = sizeof( vertices[ 0 ] ) * vertices.size();

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory );

		void *pData = nullptr;

		vulkan().device.mapMemory( stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &pData );
			std::memcpy( pData, vertices.data(), static_cast< size_t >( bufferSize ) );
		vulkan().device.unmapMemory( stagingBufferMemory );

		createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkan().vertexBuffer, vulkan().vertexBufferMemory );
		copyBuffer( stagingBuffer, vulkan().vertexBuffer, bufferSize );

		vulkan().device.destroyBuffer( stagingBuffer, nullptr );
		vulkan().device.freeMemory( stagingBufferMemory, nullptr );
	}

	void VulkanApp::createIndexBuffer()
	{
		vk::DeviceSize bufferSize = sizeof( indexBuffer[ 0 ] ) * indexBuffer.size();

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory );

		void *pData = nullptr;

		vulkan().device.mapMemory( stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &pData );
			std::memcpy( pData, indexBuffer.data(), static_cast< size_t >( bufferSize ) );
		vulkan().device.unmapMemory( stagingBufferMemory );

		createBuffer( bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkan().indexBuffer, vulkan().indexBufferMemory );
		copyBuffer( stagingBuffer, vulkan().indexBuffer, bufferSize );

		vulkan().device.destroyBuffer( stagingBuffer, nullptr );
		vulkan().device.freeMemory( stagingBufferMemory, nullptr );
	}

	void VulkanApp::createUniformBuffer()
	{
		vk::DeviceSize bufferSize = sizeof( UniformBufferObject );

		vulkan().uniformBuffers.resize( vulkan().swapChainImages.size() );
		vulkan().uniformBuffersMemory.resize( vulkan().swapChainImages.size() );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
			createBuffer( bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vulkan().uniformBuffers[ i ], vulkan().uniformBuffersMemory[ i ] );
	}

	void VulkanApp::createDescriptorPool()
	{
		std::array< vk::DescriptorPoolSize, 2 > poolSizes;
		poolSizes[ 0 ].type = vk::DescriptorType::eUniformBuffer;
		poolSizes[ 0 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
		poolSizes[ 1 ].type = vk::DescriptorType::eCombinedImageSampler;
		poolSizes[ 1 ].descriptorCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast< uint32_t >( vulkan().swapChainImages.size() );

		auto[ result, descriptorPool ] = vulkan().device.createDescriptorPool( poolInfo, nullptr );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create descriptor pool!" );

		vulkan().descriptorPool = std::move( descriptorPool );
	}

	void VulkanApp::createDescriptorSets()
	{
		std::vector< vk::DescriptorSetLayout > layouts( vulkan().swapChainImages.size(), vulkan().descriptorSetLayout );

		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = vulkan().descriptorPool;
		allocInfo.descriptorSetCount = static_cast< uint32_t >( vulkan().swapChainImages.size() );
		allocInfo.pSetLayouts = layouts.data();

		vulkan().descriptorSets.resize( vulkan().swapChainImages.size() );

		auto[ result, descriptorSets ] = vulkan().device.allocateDescriptorSets( allocInfo );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to allocate descriptor sets!" );

		vulkan().descriptorSets = std::move( descriptorSets );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
		{
			vk::DescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = vulkan().uniformBuffers[ i ];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof( UniformBufferObject );

			vk::DescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageInfo.imageView = vulkan().texture.ImageView();
			imageInfo.sampler = vulkan().texture.Sampler();

			std::array< vk::WriteDescriptorSet, 2 > descriptorWrites = {};

			descriptorWrites[ 0 ].dstSet = vulkan().descriptorSets[ i ];
			descriptorWrites[ 0 ].dstBinding = 0;
			descriptorWrites[ 0 ].dstArrayElement = 0;
			descriptorWrites[ 0 ].descriptorType = vk::DescriptorType::eUniformBuffer;
			descriptorWrites[ 0 ].descriptorCount = 1;
			descriptorWrites[ 0 ].pBufferInfo = &bufferInfo;

			descriptorWrites[ 1 ].dstSet = vulkan().descriptorSets[ i ];
			descriptorWrites[ 1 ].dstBinding = 1;
			descriptorWrites[ 1 ].dstArrayElement = 0;
			descriptorWrites[ 1 ].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descriptorWrites[ 1 ].descriptorCount = 1;
			descriptorWrites[ 1 ].pImageInfo = &imageInfo;
			descriptorWrites[ 1 ].pTexelBufferView = nullptr; // Optional

			vulkan().device.updateDescriptorSets( static_cast< uint32_t >( descriptorWrites.size() ), descriptorWrites.data(), 0, nullptr );
		}
	}

	void VulkanApp::allocateCommandBuffers()
	{
		vulkan().commandBuffers.resize( vulkan().swapChainFramebuffers.size(), nullptr );

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = vulkan().commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = static_cast< uint32_t >( vulkan().commandBuffers.size() );

		auto[ result, commandBuffers ] = vulkan().device.allocateCommandBuffers( allocInfo );

		if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );

		vulkan().commandBuffers = std::move( commandBuffers );
	}

	void VulkanApp::recordCommandBuffer( const uint32_t &imageIndex )
	{
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if ( vulkan().commandBuffers[ imageIndex ].begin( beginInfo ) != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );

		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.renderPass = vulkan().renderPass;
		renderPassInfo.framebuffer = vulkan().swapChainFramebuffers[ imageIndex ];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vulkan().swapChainExtent;

		std::array< vk::ClearValue, 2 > clearValues;
		clearValues[ 0 ].color = std::array< float, 4 >{ 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[ 1 ].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size() );
		renderPassInfo.pClearValues = clearValues.data();

		vulkan().commandBuffers[ imageIndex ].beginRenderPass( renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers );

			vk::CommandBufferInheritanceInfo inheritanceInfo;
			inheritanceInfo.renderPass = vulkan().renderPass;
			inheritanceInfo.framebuffer = vulkan().swapChainFramebuffers[ imageIndex ];

			for ( auto pMesh : m_pMeshes )
			{
				const vk::CommandBuffer &secondaryCommandBuffer = pMesh->RecordSecondaryCommandBuffers( inheritanceInfo, imageIndex );
				vulkan().commandBuffers[ imageIndex ].executeCommands( 1, &secondaryCommandBuffer );
			}

			/*for ( auto pMesh : m_pMeshes )
				pMesh->Draw( vulkan().commandBuffers[ i ], static_cast< uint32_t >( i ) );*/
			/*vkCmdBindPipeline( vulkan().commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan().graphicsPipeline );
			VkBuffer vertexBuffers[] = { vulkan().vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers( vulkan().commandBuffers[ i ], 0, 1, vertexBuffers, offsets );
			vkCmdBindIndexBuffer( vulkan().commandBuffers[ i ], vulkan().indexBuffer, 0, VK_INDEX_TYPE_UINT32 );
			vkCmdBindDescriptorSets( vulkan().commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan().pipelineLayout, 0, 1, &vulkan().descriptorSets[ i ], 0, nullptr );
			vkCmdDrawIndexed( vulkan().commandBuffers[ i ], static_cast< uint32_t >( indexBuffer.size() ), 1, 0, 0, 0 );*/
			//vkCmdDraw( vulkan().commandBuffers[ i ], static_cast< uint32_t >( vertices.size() ), 1, 0, 0 );
		vulkan().commandBuffers[ imageIndex ].endRenderPass();

		if ( vulkan().commandBuffers[ imageIndex ].end() != vk::Result::eSuccess )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );
	}

	void VulkanApp::recordCommandBuffers()
	{
		for ( size_t i = 0; i < vulkan().commandBuffers.size(); ++i )
			recordCommandBuffer( static_cast< uint32_t >( i ) );
	}

	void VulkanApp::createSyncObjects()
	{
		vulkan().imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT, nullptr );
		vulkan().renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT, nullptr );
		vulkan().inFlightFences.resize( MAX_FRAMES_IN_FLIGHT, nullptr );

		vk::SemaphoreCreateInfo semaphoreInfo;

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
		{
			if ( auto[ result, semaphore ] = vulkan().device.createSemaphore( semaphoreInfo, nullptr ); result == vk::Result::eSuccess )
				vulkan().imageAvailableSemaphores[ i ] = std::move( semaphore ); else throw std::runtime_error( "[Vulkan]Failed to create sync objects." );
			if ( auto[ result, semaphore ] = vulkan().device.createSemaphore( semaphoreInfo, nullptr ); result == vk::Result::eSuccess )
				vulkan().renderFinishedSemaphores[ i ] = std::move( semaphore ); else throw std::runtime_error( "[Vulkan]Failed to create sync objects." );
			if ( auto[ result, fence ] = vulkan().device.createFence( fenceInfo, nullptr ); result == vk::Result::eSuccess )
				vulkan().inFlightFences[ i ] = std::move( fence ); else throw std::runtime_error( "[Vulkan]Failed to create sync objects." );
		}

	}

	void VulkanApp::createBuffer( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory )
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		if ( auto[ result, buf ] = vulkan().device.createBuffer( bufferInfo, nullptr ); result == vk::Result::eSuccess )
			buffer = std::move( buf ); else throw std::runtime_error( "[Vulkan]Failed to create buffer." );

		vk::MemoryRequirements memRequirements;
		vulkan().device.getBufferMemoryRequirements( buffer, &memRequirements );

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties );

		if ( auto[ result, memory ] = vulkan().device.allocateMemory( allocInfo, nullptr ); result == vk::Result::eSuccess )
			bufferMemory = std::move( memory ); else throw std::runtime_error( "[Vulkan]Failed to allocate buffer memory!" );

		vulkan().device.bindBufferMemory( buffer, bufferMemory, 0 );
	}

	void VulkanApp::createImage( uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory, bool bCubeMap /*= false*/ )
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		if ( !bCubeMap )
			imageInfo.arrayLayers = 1;
		else
		{
			imageInfo.arrayLayers = 6;
			imageInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		}
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		if ( auto[ result, img ] = vulkan().device.createImage( imageInfo, nullptr ); result == vk::Result::eSuccess )
			image = std::move( img ); else throw std::runtime_error( "[Vulkan]Failed to create image!" );

		vk::MemoryRequirements memRequirements = vulkan().device.getImageMemoryRequirements( image );
		
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties );

		if ( auto[ result, memory ] = vulkan().device.allocateMemory( allocInfo, nullptr ); result == vk::Result::eSuccess )
			imageMemory = std::move( memory ); else throw std::runtime_error( "Failed to allocate image memory!" );

		vulkan().device.bindImageMemory( image, imageMemory, 0 );
	}

	vk::ImageView VulkanApp::createImageView( vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, vk::ImageViewType imageViewType )
	{
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = image;
		viewInfo.viewType = imageViewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;

		if ( imageViewType == vk::ImageViewType::e2D )
			viewInfo.subresourceRange.layerCount = 1;
		else if ( imageViewType == vk::ImageViewType::eCube )
			viewInfo.subresourceRange.layerCount = 6;

		if ( auto[ result, imageView ] = vulkan().device.createImageView( viewInfo, nullptr ); result == vk::Result::eSuccess )
			return imageView; else throw std::runtime_error( "[Vulkan]Failed to create image view!" );

		return nullptr;
	}

	void VulkanApp::copyBuffer( const vk::Buffer &srcBuffer, vk::Buffer &dstBuffer, vk::DeviceSize size )
	{
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
			vk::BufferCopy copyRegion = {};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;
			commandBuffer.copyBuffer( srcBuffer, dstBuffer, 1, &copyRegion );
		endSingleTimeCommands( commandBuffer );
	}

	void VulkanApp::copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t numComponents, bool bCubeMap /*= false*/ )
	{
		std::vector< vk::BufferImageCopy > bufferCopyRegions;

		if ( bCubeMap )
		{
			uint32_t offset = 0;

			for ( uint32_t face = 0; face < 6; ++face )
			{
				vk::BufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				bufferCopyRegion.imageSubresource.mipLevel = 0;
				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = width;
				bufferCopyRegion.imageExtent.height = height;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back( bufferCopyRegion );

				offset += ( width * height * numComponents );
			}
		}
		else
		{
			vk::BufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };
			bufferCopyRegions.push_back( region );
		}

		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
			commandBuffer.copyBufferToImage( buffer, image, vk::ImageLayout::eTransferDstOptimal, static_cast< uint32_t >( bufferCopyRegions.size() ), bufferCopyRegions.data() );
		endSingleTimeCommands( commandBuffer );
	}

	void VulkanApp::recreateSwapChain()
	{
		vulkan().device.waitIdle();

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createRenderPass();
		//createGraphicsPipeline();
		createDepthResources();
		createFramebuffers();

		for ( const auto &pShader : m_pShaders )
			pShader->recreateSwapChainElements();

		allocateCommandBuffers();
		recordCommandBuffers();

	}

	std::vector< const char * > VulkanApp::getRequiredExtensions()
	{
		std::vector< const char * > extensions( m_ppszReqExtensionNames, m_ppszReqExtensionNames + m_iReqExtCount );

#if VULKAN_VALIDATION_LAYERS
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

		return extensions;
	}

	VulkanApp::QueueFamilyIndices VulkanApp::findQueueFamilies( vk::PhysicalDevice &device )
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		device.getQueueFamilyProperties( &queueFamilyCount, nullptr );

		std::vector< vk::QueueFamilyProperties > queueFamilies( static_cast< size_t >( queueFamilyCount ) );
		device.getQueueFamilyProperties( &queueFamilyCount, queueFamilies.data() );

		for ( size_t index = 0; index < queueFamilies.size(); ++index )
		{
			if ( queueFamilies[ index ].queueCount > 0 )
			{
				if ( queueFamilies[ index ].queueFlags & vk::QueueFlagBits::eGraphics )
					indices.graphicsFamily = static_cast< uint32_t >( index );

				vk::Bool32 presentSupport = VK_FALSE;
				device.getSurfaceSupportKHR( static_cast< uint32_t >( index ), vulkan().surface, &presentSupport );

				if ( presentSupport == VK_TRUE )
					indices.presentFamily = static_cast< uint32_t >( index );

				if ( indices.isComplete() )
					break;
			}
		}

		return indices;
	}

	uint32_t VulkanApp::findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags properties )
	{
		vk::PhysicalDeviceMemoryProperties memProperties = vulkan().physicalDevice.getMemoryProperties();

		for ( uint32_t i = 0; i < memProperties.memoryTypeCount; ++i )
		{
			if ( typeFilter & ( 1 << i ) && ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties )
				return i;
		}

		throw std::runtime_error( "[Vulkan]Failed to find suitable memory type!" );

		// Should never get here
		return 0U;
	}

	std::shared_ptr< VulkanApp::SwapChainSupportDetails > VulkanApp::querySwapChainSupport( vk::PhysicalDevice &device )
	{
		std::shared_ptr< SwapChainSupportDetails > details = std::make_shared< SwapChainSupportDetails >();
		device.getSurfaceCapabilitiesKHR( vulkan().surface, &details->capabilities );

		uint32_t formatCount = 0;
		device.getSurfaceFormatsKHR( vulkan().surface, &formatCount, nullptr );

		if ( formatCount != 0 )
		{
			details->formats.resize( static_cast< size_t >( formatCount ) );
			device.getSurfaceFormatsKHR( vulkan().surface, &formatCount, details->formats.data() );
		}

		uint32_t presentModeCount = 0;
		device.getSurfacePresentModesKHR( vulkan().surface, &presentModeCount, nullptr );

		if ( presentModeCount != 0 )
		{
			details->presentModes.resize( static_cast< size_t >( presentModeCount ) );
			device.getSurfacePresentModesKHR( vulkan().surface, &presentModeCount, details->presentModes.data() );
		}

		return details;
	}

	vk::SurfaceFormatKHR VulkanApp::chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR > &availableFormats )
	{
		if ( availableFormats.size() == 1 && availableFormats[ 0 ].format == vk::Format::eUndefined )
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

		for ( const auto &format : availableFormats )
		{
			if ( format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
				return format;
		}

		return availableFormats[ 0 ];
	}

	// TODO: Revisit this in the future for VSync options
	vk::PresentModeKHR VulkanApp::chooseSwapPresentMode( const std::vector< vk::PresentModeKHR > &availablePresentModes )
	{
		auto selectedMode = vk::PresentModeKHR::eFifo;

		for ( const auto &presentMode : availablePresentModes )
		{
			if ( presentMode == vk::PresentModeKHR::eMailbox )
				return presentMode;
			else if ( presentMode == vk::PresentModeKHR::eImmediate )
				selectedMode = presentMode;
		}

		return selectedMode;
	}

	vk::Extent2D VulkanApp::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR &capabilities )
	{
		config &cfg = g_pEngine->GetConfig();
		vk::Extent2D actualExtent = { static_cast< uint32_t >( cfg.windowConfig.resolution_width ), static_cast< uint32_t >( cfg.windowConfig.resolution_height ) };

		actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
		actualExtent.height = std::clamp( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

		return actualExtent;
	}

	vk::ShaderModule VulkanApp::createShaderModule( const std::vector< std::byte > &code )
	{
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast< const uint32_t* >( code.data() );

		if ( auto[ result, shaderModule ] = vulkan().device.createShaderModule( createInfo, nullptr ); result == vk::Result::eSuccess )
			return shaderModule;

		return nullptr;
	}

	vk::CommandBuffer VulkanApp::beginSingleTimeCommands()
	{
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = vulkan().commandPool;
		allocInfo.commandBufferCount = 1;

		auto[ result, commandBuffer ] = vulkan().device.allocateCommandBuffers( allocInfo );

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer[ 0 ].begin( beginInfo );

		return commandBuffer[ 0 ];
	}

	void VulkanApp::endSingleTimeCommands( vk::CommandBuffer &commandBuffer )
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vulkan().graphicsQueue.submit( 1, &submitInfo, nullptr );
		vulkan().graphicsQueue.waitIdle();

		vulkan().device.freeCommandBuffers( vulkan().commandPool, 1, &commandBuffer );
	}

	void VulkanApp::transitionImageLayout( vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels, bool bCubeMap /*= false*/ )
	{
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
			vk::ImageMemoryBarrier barrier = {};
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			
			if ( newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal )
			{
				barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
				
				if ( hasStencilComponent( format ) )
					barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
			else
				barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			if ( !bCubeMap )
				barrier.subresourceRange.layerCount = 1;
			else
				barrier.subresourceRange.layerCount = 6;
			barrier.srcAccessMask = vk::AccessFlags(); // TODO
			barrier.dstAccessMask = vk::AccessFlags(); // TODO

			vk::PipelineStageFlags sourceStage, destinationStage;

			if ( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal )
			{
				barrier.srcAccessMask = vk::AccessFlags();
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
				destinationStage = vk::PipelineStageFlagBits::eTransfer;
			}
			else if ( oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal )
			{
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				sourceStage = vk::PipelineStageFlagBits::eTransfer;
				destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			}
			else if ( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal )
			{
				barrier.srcAccessMask = vk::AccessFlags();
				barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
				destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			}
			else
				throw std::invalid_argument( "[Vulkan]Unsupported layout transition!" );

			commandBuffer.pipelineBarrier(
				sourceStage, destinationStage,
				vk::DependencyFlags(),
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		endSingleTimeCommands( commandBuffer );
	}

	vk::Format VulkanApp::findSupportedFormat( const std::vector< vk::Format > &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features )
	{
		for ( const auto &format : candidates )
		{
			vk::FormatProperties props = vulkan().physicalDevice.getFormatProperties( format );

			if ( tiling == vk::ImageTiling::eLinear && ( props.linearTilingFeatures & features ) == features )
				return format;
			else if ( tiling == vk::ImageTiling::eOptimal && ( props.optimalTilingFeatures & features ) == features )
				return format;
		}

		throw std::runtime_error( "[Vulkan]Failed to find a supported format!" );
	}

	vk::Format VulkanApp::findDepthFormat()
	{
		return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	bool VulkanApp::hasStencilComponent( vk::Format format )
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	void VulkanApp::generateMipMaps( vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, bool bCubeMap /*= false*/ )
	{
		// Check if the image format supports linear blitting
		vk::FormatProperties formatProperties;
		vulkan().physicalDevice.getFormatProperties( imageFormat, &formatProperties );

		if ( !( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear ) )
			throw std::runtime_error( "[Vulkan]Texture image format does not support linear blitting!" );

		vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseArrayLayer = 0;
		if ( !bCubeMap )
			barrier.subresourceRange.layerCount = 1;
		else
			barrier.subresourceRange.layerCount = 6;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for ( uint32_t i = 1; i < mipLevels; ++i )
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(),
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			vk::ImageBlit blit = {};
			blit.srcOffsets[ 0 ] = { 0, 0, 0 };
			blit.srcOffsets[ 1 ] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			if ( !bCubeMap )
				blit.srcSubresource.layerCount = 1;
			else
				blit.srcSubresource.layerCount = 6;
			blit.dstOffsets[ 0 ] = { 0, 0, 0 };
			blit.dstOffsets[ 1 ] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			if ( !bCubeMap )
				blit.dstSubresource.layerCount = 1;
			else
				blit.dstSubresource.layerCount = 6;

			commandBuffer.blitImage(
				image, vk::ImageLayout::eTransferSrcOptimal,
				image, vk::ImageLayout::eTransferDstOptimal,
				1, &blit,
				vk::Filter::eLinear
			);

			barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(),
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			if ( mipWidth > 1 ) mipWidth /= 2;
			if ( mipHeight > 1 ) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands( commandBuffer );
	}
}