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
#include <string_view>
#include <cstdlib>

#include "testshader.hpp"
#include "camera.hpp"
#include "texturemgrvk.hpp"

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
	void VulkanThreadPool::CBJob::Execute()
	{
		static VkCommandBufferInheritanceInfo inheritanceInfo = {};

		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = GetVkRenderer_Internal().VulkanApp().vulkan().renderPass;
		inheritanceInfo.framebuffer = GetVkRenderer_Internal().VulkanApp().vulkan().swapChainFramebuffers[ m_iImageIndex ];

		m_Result = GetVkRenderer_Internal().VulkanApp().m_pMeshes[ m_iMeshIndex ]->RecordSecondaryCommandBuffers( inheritanceInfo, m_iImageIndex );
	}

	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 mvp;
	};

	static VkVertexInputBindingDescription getVertexBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof( Vertex );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array< VkVertexInputAttributeDescription, 4 > getVertexAttributeDescriptions()
	{
		std::array< VkVertexInputAttributeDescription, 4 > attributeDescriptions = {};

		attributeDescriptions[ 0 ].binding = 0;
		attributeDescriptions[ 0 ].location = 0;
		attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

		attributeDescriptions[ 1 ].binding = 0;
		attributeDescriptions[ 1 ].location = 1;
		attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

		attributeDescriptions[ 2 ].binding = 0;
		attributeDescriptions[ 2 ].location = 2;
		attributeDescriptions[ 2 ].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );

		attributeDescriptions[ 3 ].binding = 0;
		attributeDescriptions[ 3 ].location = 3;
		attributeDescriptions[ 3 ].format =  VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[ 3 ].offset = offsetof( Vertex, normal );

		return attributeDescriptions;
	}

	/*std::vector< Vertex > vertices =
	{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
	};*/

	VulkanApp::VulkanApp()
	{
	}

	VulkanApp::~VulkanApp()
	{
		cleanup();
	}

	bool VulkanApp::initVulkan()
	{
		m_ThreadPool.InitThreads();

		std::cout << "hardware_concurrency: " << std::thread::hardware_concurrency() << std::endl;

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

		//m_pTestModel = new ModelVK;
		//m_pTestModel->SetModelMatrix( glm::translate( Vector3f( 0.0f, 2.0f, 0.0f ) ) );
		//m_pTestModel->LoadModel( string( GAME_DIR ) + "models/axis.amdl" );

		m_pTestModel2 = new ModelVK;
		m_pTestModel2->SetModelMatrix( glm::translate( Vector3f( 0.0f, 2.0f, 0.0f ) ) );
		m_pTestModel2->LoadModel( PATHS::GAME / "models" / "chalet_mat.amdl" );

		std::vector< Vertex > skyVerts;
		skyVerts.resize( skyboxVertices.size() );
		for ( size_t i = 0; i < skyboxVertices.size(); ++i )
			skyVerts[ i ].pos = skyboxVertices[ i ];

		m_pSkyboxMaterial = new MaterialVK( PATHS::GAME / "materials" / "skybox" / "default_sky.amat" );
		m_pSkyboxTest = new MeshVK( skyVerts, {}, m_pSkyboxMaterial );
		m_pTestMaterial = new MaterialVK( PATHS::GAME / "materials" / "test.amat" );

		allocateCommandBuffers();
		//recordCommandBuffers();
		createSyncObjects();

		//m_pTestMaterial = new MaterialVK( string( GAME_DIR ) + "materials/cube/BasicMaterial.amat" );
		//m_pTestMaterial = new MaterialVK( string( GAME_DIR ) + "materials/chalet/chalet.amat" );
		//m_pTestMesh = new MeshVK( vertices, indexBuffer, m_pTestMaterial );

		return true;
	}

	void VulkanApp::cleanup()
	{
		m_ThreadPool.TerminateThreads();

		if ( vulkan().device != VK_NULL_HANDLE )
			vkDeviceWaitIdle( vulkan().device );

		TextureMgrVK::Shutdown();

		/*if ( m_pTestMesh )
		{
			delete m_pTestMesh;
			m_pTestMesh = nullptr;
		}

		delete m_pTestMaterial;
		m_pTestMaterial = nullptr;*/

		if ( m_pTestMaterial )
		{
			delete m_pTestMaterial;
			m_pTestMaterial = nullptr;
		}

		for ( auto pMesh : m_pTestMeshes )
			delete pMesh;

		m_pTestMeshes.clear();

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

		for ( size_t i = 0; i < vulkan().uniformBuffers.size(); ++i )
		{
			vkDestroyBuffer( vulkan().device, vulkan().uniformBuffers[ i ], nullptr );
			vkFreeMemory( vulkan().device, vulkan().uniformBuffersMemory[ i ], nullptr );
		}

		vulkan().uniformBuffers.clear();
		vulkan().uniformBuffersMemory.clear();

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
		{
			if ( vulkan().renderFinishedSemaphores.size() > 0 )
			{
				if ( vulkan().renderFinishedSemaphores[ i ] != VK_NULL_HANDLE )
					vkDestroySemaphore( vulkan().device, vulkan().renderFinishedSemaphores[ i ], nullptr );
			}

			if ( vulkan().imageAvailableSemaphores.size() > 0 )
			{
				if ( vulkan().imageAvailableSemaphores[ i ] != VK_NULL_HANDLE )
					vkDestroySemaphore( vulkan().device, vulkan().imageAvailableSemaphores[ i ], nullptr );
			}

			if ( vulkan().inFlightFences.size() > 0 )
			{
				if ( vulkan().inFlightFences[ i ] != VK_NULL_HANDLE )
					vkDestroyFence( vulkan().device, vulkan().inFlightFences[ i ], nullptr );
			}
		}

		vulkan().renderFinishedSemaphores.clear();
		vulkan().imageAvailableSemaphores.clear();
		vulkan().inFlightFences.clear();

		if ( vulkan().commandPool != VK_NULL_HANDLE )
		{
			vkDestroyCommandPool( vulkan().device, vulkan().commandPool, nullptr );
			vulkan().commandPool = VK_NULL_HANDLE;
		}

		if ( vulkan().device != VK_NULL_HANDLE )
		{
			vkDestroyDevice( vulkan().device, nullptr );
			vulkan().device = VK_NULL_HANDLE;

			vulkan().graphicsQueue = VK_NULL_HANDLE; // This is cleaned up when the device is destroyed
			vulkan().presentQueue = VK_NULL_HANDLE; // This is cleaned up when the device is destroyed
		}

#if VULKAN_VALIDATION_LAYERS
		if ( vulkan().debugCallback != VK_NULL_HANDLE )
		{
			DestroyDebugUtilsMessengerEXT( vulkan().instance, vulkan().debugCallback, nullptr );
			vulkan().debugCallback = VK_NULL_HANDLE;
		}
#endif

		if ( vulkan().surface != VK_NULL_HANDLE )
		{
			vkDestroySurfaceKHR( vulkan().instance, vulkan().surface, nullptr );
			vulkan().surface = VK_NULL_HANDLE;
		}

		if ( vulkan().instance != VK_NULL_HANDLE )
		{
			vkDestroyInstance( vulkan().instance, nullptr );
			vulkan().instance = VK_NULL_HANDLE;
			vulkan().physicalDevice = VK_NULL_HANDLE; // This is cleaned up when the instance is destroyed
		}

		if ( m_ppszReqExtensionNames )
		{
			delete[] m_ppszReqExtensionNames;
			m_ppszReqExtensionNames = nullptr;
		}
	}

	void VulkanApp::cleanupSwapChain()
	{
		if ( vulkan().depthImageView != VK_NULL_HANDLE )
		{
			vkDestroyImageView( vulkan().device, vulkan().depthImageView, nullptr );
			vulkan().depthImageView = VK_NULL_HANDLE;
		}

		if ( vulkan().depthImage != VK_NULL_HANDLE )
		{
			vkDestroyImage( vulkan().device, vulkan().depthImage, nullptr );
			vulkan().depthImage = VK_NULL_HANDLE;
		}

		if ( vulkan().depthImageMemory != VK_NULL_HANDLE )
		{
			vkFreeMemory( vulkan().device, vulkan().depthImageMemory, nullptr );
			vulkan().depthImageMemory = VK_NULL_HANDLE;
		}

		for ( auto &framebuffer : vulkan().swapChainFramebuffers )
		{
			if ( framebuffer != VK_NULL_HANDLE )
				vkDestroyFramebuffer( vulkan().device, framebuffer, nullptr );
		}

		vulkan().swapChainFramebuffers.clear();

		if ( vulkan().commandBuffers.size() > 0 )
			vkFreeCommandBuffers( vulkan().device, vulkan().commandPool, static_cast< uint32_t >( vulkan().commandBuffers.size() ), vulkan().commandBuffers.data() );

		vulkan().commandBuffers.clear();

		if ( vulkan().renderPass != VK_NULL_HANDLE )
		{
			vkDestroyRenderPass( vulkan().device, vulkan().renderPass, nullptr );
			vulkan().renderPass = VK_NULL_HANDLE;
		}

		for ( auto &imageView : vulkan().swapChainImageViews )
		{
			if ( imageView != VK_NULL_HANDLE )
				vkDestroyImageView( vulkan().device, imageView, nullptr );
		}

		vulkan().swapChainImageViews.clear();
		vulkan().swapChainImages.clear();

		if ( vulkan().swapChain != VK_NULL_HANDLE )
		{
			vkDestroySwapchainKHR( vulkan().device, vulkan().swapChain, nullptr );
			vulkan().swapChain = VK_NULL_HANDLE;
		}
	}

	void VulkanApp::drawFrame()
	{
		static uint32_t imageIndex = 0;
		static size_t currentFrame = 0;

		if ( vulkan().bMinimized )
			return;

		if ( vulkan().bFrameBufferResized )
		{
			vulkan().bFrameBufferResized = false;
			recreateSwapChain();
			return;
		}

		vkWaitForFences( vulkan().device, 1, &vulkan().inFlightFences[ currentFrame ], VK_TRUE, std::numeric_limits< uint64_t >::max() );

		if ( auto result = vkAcquireNextImageKHR( vulkan().device, vulkan().swapChain, std::numeric_limits< uint64_t >::max(), vulkan().imageAvailableSemaphores[ currentFrame ], VK_NULL_HANDLE, &imageIndex ); result != VK_SUCCESS )
		{
			if ( result == VK_ERROR_OUT_OF_DATE_KHR )
			{
				recreateSwapChain();
				return;
			}
			else if ( result != VK_SUBOPTIMAL_KHR )
			{
				stprintf( "[Vulkan]Failed to acquire swap chain image!\n" );
				return;
			}
		}

		//updateUniformBuffer( imageIndex );

		recordCommandBuffer( imageIndex );

		VkSemaphore waitSemaphores[] = { vulkan().imageAvailableSemaphores[ currentFrame ] };
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vulkan().commandBuffers[ imageIndex ];

		VkSemaphore signalSemaphores[] = { vulkan().renderFinishedSemaphores[ currentFrame ] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences( vulkan().device, 1, &vulkan().inFlightFences[ currentFrame ] );

		if ( vkQueueSubmit( vulkan().graphicsQueue, 1, &submitInfo, vulkan().inFlightFences[ currentFrame ] ) != VK_SUCCESS )
		{
			stprintf( "[Vulkan]Queue submit failed!\n" );
			return;
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vulkan().swapChain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		if ( vkQueuePresentKHR( vulkan().presentQueue, &presentInfo ) != VK_SUCCESS )
		{
			recreateSwapChain();
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

	VkResult VulkanApp::CreateDebugUtilsMessengerEXT( VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pCallback )
	{
		auto func = ( PFN_vkCreateDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		return ( func ) ? func( instance, reinterpret_cast< const VkDebugUtilsMessengerCreateInfoEXT* >( pCreateInfo ), reinterpret_cast< const VkAllocationCallbacks* >( pAllocator ), reinterpret_cast< VkDebugUtilsMessengerEXT* >( pCallback ) ) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VulkanApp::DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator )
	{
		auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if ( func != nullptr )
			func( instance, callback, reinterpret_cast< const VkAllocationCallbacks* >( pAllocator ) );
	}

	void VulkanApp::checkValidationLayerSupport()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

		std::vector< VkLayerProperties > availableLayers( static_cast< size_t >( layerCount ) );
		vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

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
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "AMEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.pEngineName = "AMEngine";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
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

		if ( vkCreateInstance( &createInfo, nullptr, &vulkan().instance ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create instance." );

		// Query available extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

		vulkan().extensions.resize( static_cast< size_t >( extensionCount ) );
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, vulkan().extensions.data() );

		stprintf( "[Vulkan]Available Extensions:\n" );

		for ( const auto &extension : vulkan().extensions )
			stprintf( "\t%s\n", extension.extensionName );

		stprintf( "\n" );
	}

#if VULKAN_VALIDATION_LAYERS
	void VulkanApp::setupDebugCallback()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		if ( CreateDebugUtilsMessengerEXT( vulkan().instance, &createInfo, nullptr, &vulkan().debugCallback ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to setup debug callback." );
	}
#endif

	void VulkanApp::createSurface()
	{
		if ( SDL_Vulkan_CreateSurface( GetVkRenderer_Internal().GetWindow(), vulkan().instance, &vulkan().surface ) != SDL_TRUE )
			throw std::runtime_error( "[Vulkan]Failed to create window surface." );
	}

	void VulkanApp::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices( vulkan().instance, &deviceCount, nullptr );

		if ( deviceCount == 0 )
			throw std::runtime_error( "[Vulkan]No physical device found." );

		std::vector< VkPhysicalDevice > devices( static_cast< size_t >( deviceCount ) );
		vkEnumeratePhysicalDevices( vulkan().instance, &deviceCount, devices.data() );

		auto rateSuitability = []( VkPhysicalDevice &device )
		{
			uint32_t iScore = 1;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties( device, &deviceProperties );

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

			if ( deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
				iScore += 1000;

			iScore += deviceProperties.limits.maxImageDimension2D;

			if ( !deviceFeatures.geometryShader )
				return static_cast< uint32_t >( 0 );

			if ( !deviceFeatures.textureCompressionBC )
				return static_cast< uint32_t >( 0 );

			return iScore;
		};

		// Ordered map that automatically sorts device candidates by increasing score
		std::multimap< uint32_t, VkPhysicalDevice > candidates;

		for ( VkPhysicalDevice &device : devices )
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

	bool VulkanApp::isDeviceSuitable( VkPhysicalDevice &device )
	{
		auto checkDeviceExtensionSupport = [ &, this ]()
		{
			uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

			std::vector< VkExtensionProperties > availableExtensions( static_cast< size_t >( extensionCount ) );
			vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

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

		std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
		std::set< uint32_t > uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for ( auto queueFamily : uniqueQueueFamilies )
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back( queueCreateInfo );
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.textureCompressionBC = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
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

		if ( vkCreateDevice( vulkan().physicalDevice, &createInfo, nullptr, &vulkan().device ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create logical device." );

		vkGetDeviceQueue( vulkan().device, indices.graphicsFamily, 0, &vulkan().graphicsQueue );
		vkGetDeviceQueue( vulkan().device, indices.presentFamily, 0, &vulkan().presentQueue );
	}

	void VulkanApp::createSwapChain()
	{
		auto swapChainSupport = querySwapChainSupport( vulkan().physicalDevice );

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport->formats );
		VkPresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport->presentModes );
		VkExtent2D extent = chooseSwapExtent( swapChainSupport->capabilities );

		uint32_t imageCount = swapChainSupport->capabilities.maxImageCount > 0 ?
			std::min( swapChainSupport->capabilities.minImageCount + 1, swapChainSupport->capabilities.maxImageCount ) :
			swapChainSupport->capabilities.minImageCount + 1;

		VkSwapchainCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = vulkan().surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto indices = findQueueFamilies( vulkan().physicalDevice );
		uint32_t queueFamilyIndices[] =
		{
			indices.graphicsFamily,
			indices.presentFamily
		};

		if ( indices.graphicsFamily != indices.presentFamily )
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport->capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // TODO: Revisit this for window blending
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;

		if ( vkCreateSwapchainKHR( vulkan().device, &createInfo, nullptr, &vulkan().swapChain ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create swap chain." );

		vkGetSwapchainImagesKHR( vulkan().device, vulkan().swapChain, &imageCount, nullptr );
		vulkan().swapChainImages.resize( imageCount, VK_NULL_HANDLE );
		vkGetSwapchainImagesKHR( vulkan().device, vulkan().swapChain, &imageCount, vulkan().swapChainImages.data() );

		vulkan().swapChainImageFormat = surfaceFormat.format;
		vulkan().swapChainExtent = extent;
	}

	void VulkanApp::createImageViews()
	{
		vulkan().swapChainImageViews.resize( vulkan().swapChainImages.size(), nullptr );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); i++ )
			vulkan().swapChainImageViews[ i ] = createImageView( vulkan().swapChainImages[ i ], vulkan().swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D );
	}

	void VulkanApp::createRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = vulkan().swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef;
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array< VkAttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if ( vkCreateRenderPass( vulkan().device, &renderPassInfo, nullptr, &vulkan().renderPass ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create render pass." );
	}

	void VulkanApp::createFramebuffers()
	{
		vulkan().swapChainFramebuffers.resize( vulkan().swapChainImageViews.size(), nullptr );

		for ( size_t i = 0; i < vulkan().swapChainFramebuffers.size(); ++i )
		{
			std::array< VkImageView, 2 > attachments = { vulkan().swapChainImageViews[ i ], vulkan().depthImageView };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = vulkan().renderPass;
			framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = vulkan().swapChainExtent.width;
			framebufferInfo.height = vulkan().swapChainExtent.height;
			framebufferInfo.layers = 1;

			if ( vkCreateFramebuffer( vulkan().device, &framebufferInfo, nullptr, &vulkan().swapChainFramebuffers[ i ] ) != VK_SUCCESS )
				throw std::runtime_error( "[Vulkan]Failed to create framebuffers." );
		}
	}

	void VulkanApp::createCommandPool()
	{
		auto queueFamilyIndices = findQueueFamilies( vulkan().physicalDevice );

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		if ( vkCreateCommandPool( vulkan().device, &poolInfo, nullptr, &vulkan().commandPool ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create command pool." );
	}

	void VulkanApp::createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();
		createImage( vulkan().swapChainExtent.width, vulkan().swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan().depthImage, vulkan().depthImageMemory );
		vulkan().depthImageView = createImageView( vulkan().depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, VK_IMAGE_VIEW_TYPE_2D );
		transitionImageLayout( vulkan().depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 );
	}

	void VulkanApp::createUniformBuffer()
	{
		VkDeviceSize bufferSize = sizeof( UniformBufferObject );

		vulkan().uniformBuffers.resize( vulkan().swapChainImages.size() );
		vulkan().uniformBuffersMemory.resize( vulkan().swapChainImages.size() );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); ++i )
			createBuffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vulkan().uniformBuffers[ i ], vulkan().uniformBuffersMemory[ i ] );
	}

	void VulkanApp::allocateCommandBuffers()
	{
		vulkan().commandBuffers.resize( vulkan().swapChainFramebuffers.size(), nullptr );

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vulkan().commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast< uint32_t >( vulkan().commandBuffers.size() );

		if ( vkAllocateCommandBuffers( vulkan().device, &allocInfo, vulkan().commandBuffers.data() ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );
	}

	void VulkanApp::recordCommandBuffer( const uint32_t &imageIndex )
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if ( vkBeginCommandBuffer( vulkan().commandBuffers[ imageIndex ], &beginInfo ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vulkan().renderPass;
		renderPassInfo.framebuffer = vulkan().swapChainFramebuffers[ imageIndex ];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vulkan().swapChainExtent;

		std::array< VkClearValue, 2 > clearValues;
		clearValues[ 0 ].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[ 1 ].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size() );
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass( vulkan().commandBuffers[ imageIndex ], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS );

			constexpr const bool MultiThread = true;

			if constexpr ( MultiThread )
			{
				for ( size_t i = 0; i < m_pMeshes.size(); ++i )
					m_ThreadPool.AddJob( { imageIndex, i } );

				m_ThreadPool.AssignJobs();
				m_ThreadPool.ExecuteJobs();
				m_ThreadPool.WaitIdle();

				const auto &jobs = m_ThreadPool.GetJobs();
				for ( const auto &job : jobs )
					vkCmdExecuteCommands( vulkan().commandBuffers[ imageIndex ], 1, job.GetResult() );

				m_ThreadPool.ClearJobs();
			}
			else
			{
				VkCommandBufferInheritanceInfo inheritanceInfo = {};
				inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceInfo.renderPass = vulkan().renderPass;
				inheritanceInfo.framebuffer = vulkan().swapChainFramebuffers[ imageIndex ];

				for ( auto pMesh : m_pMeshes )
				{
					const VkCommandBuffer &secondaryCommandBuffer = pMesh->RecordSecondaryCommandBuffers( inheritanceInfo, imageIndex );
					vkCmdExecuteCommands( vulkan().commandBuffers[ imageIndex ], 1, &secondaryCommandBuffer );
				}
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
		vkCmdEndRenderPass( vulkan().commandBuffers[ imageIndex ] );

		if ( vkEndCommandBuffer( vulkan().commandBuffers[ imageIndex ] ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create command buffers." );
	}

	void VulkanApp::recordCommandBuffers()
	{
		for ( size_t i = 0; i < vulkan().commandBuffers.size(); ++i )
			recordCommandBuffer( static_cast< uint32_t >( i ) );
	}

	void VulkanApp::createSyncObjects()
	{
		vulkan().imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );
		vulkan().renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );
		vulkan().inFlightFences.resize( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
		{
			if ( vkCreateSemaphore( vulkan().device, &semaphoreInfo, nullptr, &vulkan().imageAvailableSemaphores[ i ] ) != VK_SUCCESS )
				throw std::runtime_error( "[Vulkan]Failed to create sync objects." );

			if ( vkCreateSemaphore( vulkan().device, &semaphoreInfo, nullptr, &vulkan().renderFinishedSemaphores[ i ] ) != VK_SUCCESS )
				throw std::runtime_error( "[Vulkan]Failed to create sync objects." );

			if ( vkCreateFence( vulkan().device, &fenceInfo, nullptr, &vulkan().inFlightFences[ i ] ) != VK_SUCCESS )
				throw std::runtime_error( "[Vulkan]Failed to create sync objects." );
		}

	}

	void VulkanApp::createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory )
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ( vkCreateBuffer( vulkan().device, &bufferInfo, nullptr, &buffer ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create buffer." );

		VkMemoryRequirements memRequirements = {};
		vkGetBufferMemoryRequirements( vulkan().device, buffer, &memRequirements );

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties );

		if ( vkAllocateMemory( vulkan().device, &allocInfo, nullptr, &bufferMemory ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to allocate buffer memory!" );

		vkBindBufferMemory( vulkan().device, buffer, bufferMemory, 0 );
	}

	void VulkanApp::createImage( uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory, bool bCubeMap /*= false*/ )
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		if ( !bCubeMap )
			imageInfo.arrayLayers = 1;
		else
		{
			imageInfo.arrayLayers = 6;
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ( vkCreateImage( vulkan().device, &imageInfo, nullptr, &image ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create image!" );

		VkMemoryRequirements memRequirements = {};
		vkGetImageMemoryRequirements( vulkan().device, image, &memRequirements );
		
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties );

		if ( vkAllocateMemory( vulkan().device, &allocInfo, nullptr, &imageMemory ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate image memory!" );

		vkBindImageMemory( vulkan().device, image, imageMemory, 0 );
	}

	VkImageView VulkanApp::createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType imageViewType )
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.image = image;
		viewInfo.viewType = imageViewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;

		if ( imageViewType == VK_IMAGE_VIEW_TYPE_2D )
			viewInfo.subresourceRange.layerCount = 1;
		else if ( imageViewType == VK_IMAGE_VIEW_TYPE_CUBE )
			viewInfo.subresourceRange.layerCount = 6;

		VkImageView imageView = VK_NULL_HANDLE;
		if ( vkCreateImageView( vulkan().device, &viewInfo, nullptr, &imageView ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create image view!" );

		return imageView;
	}

	void VulkanApp::copyBuffer( const VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize size )
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;
			vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );
		endSingleTimeCommands( commandBuffer );
	}

	void VulkanApp::copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t numComponents, bool bCubeMap /*= false*/ )
	{
		std::vector< VkBufferImageCopy > bufferCopyRegions;

		if ( bCubeMap )
		{
			uint32_t offset = 0;

			for ( uint32_t face = 0; face < 6; ++face )
			{
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };
			bufferCopyRegions.push_back( region );
		}

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
			vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast< uint32_t >( bufferCopyRegions.size() ), bufferCopyRegions.data() );
		endSingleTimeCommands( commandBuffer );
	}

	void VulkanApp::recreateSwapChain()
	{
		vkDeviceWaitIdle( vulkan().device );

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

	VulkanApp::QueueFamilyIndices VulkanApp::findQueueFamilies( VkPhysicalDevice &device )
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

		std::vector< VkQueueFamilyProperties > queueFamilies( static_cast< size_t >( queueFamilyCount ) );
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

		for ( size_t index = 0; index < queueFamilies.size(); ++index )
		{
			if ( queueFamilies[ index ].queueCount > 0 )
			{
				if ( queueFamilies[ index ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
					indices.graphicsFamily = static_cast< uint32_t >( index );

				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR( device, static_cast< uint32_t >( index ), vulkan().surface, &presentSupport );

				if ( presentSupport == VK_TRUE )
					indices.presentFamily = static_cast< uint32_t >( index );

				if ( indices.isComplete() )
					break;
			}
		}

		return indices;
	}

	uint32_t VulkanApp::findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
	{
		VkPhysicalDeviceMemoryProperties memProperties = {};
		vkGetPhysicalDeviceMemoryProperties( vulkan().physicalDevice, &memProperties );

		for ( uint32_t i = 0; i < memProperties.memoryTypeCount; ++i )
		{
			if ( typeFilter & ( 1 << i ) && ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties )
				return i;
		}

		throw std::runtime_error( "[Vulkan]Failed to find suitable memory type!" );

		// Should never get here
		return 0U;
	}

	std::shared_ptr< VulkanApp::SwapChainSupportDetails > VulkanApp::querySwapChainSupport( VkPhysicalDevice &device )
	{
		std::shared_ptr< SwapChainSupportDetails > details = std::make_shared< SwapChainSupportDetails >();
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, vulkan().surface, &details->capabilities );

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( device, vulkan().surface, &formatCount, nullptr );

		if ( formatCount != 0 )
		{
			details->formats.resize( static_cast< size_t >( formatCount ) );
			vkGetPhysicalDeviceSurfaceFormatsKHR( device, vulkan().surface, &formatCount, details->formats.data() );
		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR( device, vulkan().surface, &presentModeCount, nullptr );

		if ( presentModeCount != 0 )
		{
			details->presentModes.resize( static_cast< size_t >( presentModeCount ) );
			vkGetPhysicalDeviceSurfacePresentModesKHR( device, vulkan().surface, &presentModeCount, details->presentModes.data() );
		}

		return details;
	}

	VkSurfaceFormatKHR VulkanApp::chooseSwapSurfaceFormat( const std::vector< VkSurfaceFormatKHR > &availableFormats )
	{
		if ( availableFormats.size() == 1 && availableFormats[ 0 ].format == VK_FORMAT_UNDEFINED )
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

		for ( const auto &format : availableFormats )
		{
			if ( format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
				return format;
		}

		return availableFormats[ 0 ];
	}

	// TODO: Revisit this in the future for VSync options
	VkPresentModeKHR VulkanApp::chooseSwapPresentMode( const std::vector< VkPresentModeKHR > &availablePresentModes )
	{
		auto selectedMode = VK_PRESENT_MODE_FIFO_KHR;

		for ( const auto &presentMode : availablePresentModes )
		{
			if ( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
				return presentMode;
			else if ( presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
				selectedMode = presentMode;
		}

		return selectedMode;
	}

	VkExtent2D VulkanApp::chooseSwapExtent( const VkSurfaceCapabilitiesKHR &capabilities )
	{
		config &cfg = g_pEngine->GetConfig();
		VkExtent2D actualExtent = { static_cast< uint32_t >( cfg.windowConfig.resolution_width ), static_cast< uint32_t >( cfg.windowConfig.resolution_height ) };

		actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
		actualExtent.height = std::clamp( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

		return actualExtent;
	}

	VkShaderModule VulkanApp::createShaderModule( const std::vector< std::byte > &code )
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast< const uint32_t* >( code.data() );

		VkShaderModule shaderModule = VK_NULL_HANDLE;
		if ( vkCreateShaderModule( vulkan().device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to create shader module!" );

		return shaderModule;
	}

	VkCommandBuffer VulkanApp::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vulkan().commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		if ( vkAllocateCommandBuffers( vulkan().device, &allocInfo, &commandBuffer ) != VK_SUCCESS )
			throw std::runtime_error( "[Vulkan]Failed to allocate single use command buffer!" );
		else
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer( commandBuffer, &beginInfo );

			return commandBuffer;
		}

		return nullptr;
	}

	void VulkanApp::endSingleTimeCommands( VkCommandBuffer &commandBuffer )
	{
		vkEndCommandBuffer( commandBuffer );

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit( vulkan().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( vulkan().graphicsQueue );

		vkFreeCommandBuffers( vulkan().device, vulkan().commandPool, 1, &commandBuffer );
	}

	void VulkanApp::transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, bool bCubeMap /*= false*/ )
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			
			if ( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				
				if ( hasStencilComponent( format ) )
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			if ( !bCubeMap )
				barrier.subresourceRange.layerCount = 1;
			else
				barrier.subresourceRange.layerCount = 6;
			barrier.srcAccessMask = 0; // TODO
			barrier.dstAccessMask = 0; // TODO

			VkPipelineStageFlags sourceStage, destinationStage;

			if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
				throw std::invalid_argument( "[Vulkan]Unsupported layout transition!" );

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr, 
				0, nullptr, 
				1, &barrier );

		endSingleTimeCommands( commandBuffer );
	}

	VkFormat VulkanApp::findSupportedFormat( const std::vector< VkFormat > &candidates, VkImageTiling tiling, VkFormatFeatureFlags features )
	{
		for ( const auto &format : candidates )
		{
			VkFormatProperties props = {};
			vkGetPhysicalDeviceFormatProperties( vulkan().physicalDevice, format, &props );

			if ( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features )
				return format;
			else if ( tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features )
				return format;
		}

		throw std::runtime_error( "[Vulkan]Failed to find a supported format!" );
	}

	VkFormat VulkanApp::findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool VulkanApp::hasStencilComponent( VkFormat format )
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void VulkanApp::generateMipMaps( VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, bool bCubeMap /*= false*/ )
	{
		// Check if the image format supports linear blitting
		VkFormatProperties formatProperties = {};
		vkGetPhysicalDeviceFormatProperties( vulkan().physicalDevice, imageFormat, &formatProperties );

		if ( !( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ) )
			throw std::runtime_error( "[Vulkan]Texture image format does not support linear blitting!" );

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit = {};
			blit.srcOffsets[ 0 ] = { 0, 0, 0 };
			blit.srcOffsets[ 1 ] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			if ( !bCubeMap )
				blit.srcSubresource.layerCount = 1;
			else
				blit.srcSubresource.layerCount = 6;
			blit.dstOffsets[ 0 ] = { 0, 0, 0 };
			blit.dstOffsets[ 1 ] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			if ( !bCubeMap )
				blit.dstSubresource.layerCount = 1;
			else
				blit.dstSubresource.layerCount = 6;

			vkCmdBlitImage(
				commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			if ( mipWidth > 1 ) mipWidth /= 2;
			if ( mipHeight > 1 ) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands( commandBuffer );
	}
}