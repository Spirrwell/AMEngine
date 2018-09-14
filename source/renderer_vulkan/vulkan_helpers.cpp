#include "vulkan_helpers.hpp"
#include "renderer_vulkan.hpp"
#include "engine/config.hpp"

#include "SDL_vulkan.h"
#include "mathdefs.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <array>
#include <map>
#include <set>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

namespace vkApp
{
	struct Vertex
	{
		Vector2f pos;
		Vector3f color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof( Vertex );
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array< VkVertexInputAttributeDescription, 2 > getAttributeDescriptions()
		{
			std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions = {};

			attributeDescriptions[ 0 ].binding = 0;
			attributeDescriptions[ 0 ].location = 0;
			attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

			attributeDescriptions[ 1 ].binding = 0;
			attributeDescriptions[ 1 ].location = 1;
			attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

			return attributeDescriptions;
		}
	};

	std::vector< Vertex > vertices =
	{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
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

	const char *VulkanApp::s_pszErrors[ VKAPP_ERROR_COUNT ] =
	{
		"None",
		"Failed to load required extensions from SDL.",
		"Failed to create instance.",
#if VULKAN_VALIDATION_LAYERS
		"Failed to get validation layers.",
		"Failed to set up debug callback.",
#endif
		"Failed to create window surface.",
		"No physical device found.",
		"Failed to create logical device.",
		"Failed to create swap chain.",
		"Failed to create image views.",
		"Failed to create render pass.",
		"Failed to create graphics pipeline.",
		"Failed to create framebuffers.",
		"Failed to create command pool.",
		"Failed to create sync objects."
	};

	VulkanApp::VulkanApp()
	{
	}

	VulkanApp::~VulkanApp()
	{
		cleanup();
	}

	bool VulkanApp::initVulkan()
	{
		if ( !loadExtensionsFromSDL() )
			return false;

#if VULKAN_VALIDATION_LAYERS
		if ( !checkValidationLayerSupport() )
			return false;
#endif
		if ( !createInstance() )
			return false;

#if VULKAN_VALIDATION_LAYERS
		if ( !setupDebugCallback() )
			return false;
#endif

		if ( !createSurface() )
			return false;

		if ( !pickPhysicalDevice() )
			return false;

		if ( !createLogicalDevice() )
			return false;

		if ( !createSwapChain() )
			return false;

		if ( !createImageViews() )
			return false;

		if ( !createRenderPass() )
			return false;

		if ( !createGraphicsPipeline() )
			return false;

		if ( !createFramebuffers() )
			return false;

		if ( !createCommandPool() )
			return false;

		if ( !createCommandBuffers() )
			return false;

		if ( !createSyncObjects() )
			return false;

		return true;
	}

	void VulkanApp::cleanup()
	{
		if ( vulkan().device != VK_NULL_HANDLE )
			vkDeviceWaitIdle( vulkan().device );

		cleanupSwapChain();

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
		for ( auto &framebuffer : vulkan().swapChainFramebuffers )
		{
			if ( framebuffer != VK_NULL_HANDLE )
				vkDestroyFramebuffer( vulkan().device, framebuffer, nullptr );
		}

		vulkan().swapChainFramebuffers.clear();

		if ( vulkan().commandBuffers.size() > 0 )
			vkFreeCommandBuffers( vulkan().device, vulkan().commandPool, static_cast< uint32_t >( vulkan().commandBuffers.size() ), vulkan().commandBuffers.data() );

		vulkan().commandBuffers.clear();

		if ( vulkan().graphicsPipeline != VK_NULL_HANDLE )
		{
			vkDestroyPipeline( vulkan().device, vulkan().graphicsPipeline, nullptr );
			vulkan().graphicsPipeline = VK_NULL_HANDLE;
		}

		if ( vulkan().pipelineLayout != VK_NULL_HANDLE )
		{
			vkDestroyPipelineLayout( vulkan().device, vulkan().pipelineLayout, nullptr );
			vulkan().pipelineLayout = VK_NULL_HANDLE;
		}

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

		vkWaitForFences( vulkan().device, 1, &vulkan().inFlightFences[ currentFrame ], VK_TRUE, std::numeric_limits< uint64_t >::max() );

		VkResult result = vkAcquireNextImageKHR( vulkan().device, vulkan().swapChain, std::numeric_limits< uint64_t >::max(), vulkan().imageAvailableSemaphores[ currentFrame ], VK_NULL_HANDLE, &imageIndex );

		if ( result == VK_ERROR_OUT_OF_DATE_KHR )
		{
			recreateSwapChain();
			return;
		}
		else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
		{
			stprintf( "[Vulkan]Failed to acquire swap chain image.\n" );
			return;
		}

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
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { vulkan().swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR( vulkan().presentQueue, &presentInfo );

		if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkan().bFrameBufferResized )
		{
			vulkan().bFrameBufferResized = false;
			recreateSwapChain();
			return;
		}
		else if ( result != VK_SUCCESS )
		{
			stprintf( "[Vulkan]Failed to present swap chain image.\n" );
			return;
		}
		
		currentFrame = ( currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
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
		auto func = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		return ( func ) ? func( instance, pCreateInfo, pAllocator, pCallback ) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VulkanApp::DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator )
	{
		auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if ( func != nullptr )
			func( instance, callback, pAllocator );
	}

	bool VulkanApp::checkValidationLayerSupport()
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
			{
				setError( VKAPP_ERROR_VALIDATION_LAYERS );
				return false;
			}
		}

		return true;
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

	bool VulkanApp::loadExtensionsFromSDL()
	{
		unsigned int extensionCount = 0;
		SDL_Vulkan_GetInstanceExtensions( GetVkRenderer_Internal().GetWindow(), &extensionCount, nullptr );

		if ( extensionCount == 0)
			return false;

		m_ppszReqExtensionNames = new const char *[ extensionCount ];
		SDL_Vulkan_GetInstanceExtensions( GetVkRenderer_Internal().GetWindow(), &extensionCount, m_ppszReqExtensionNames );

		m_iReqExtCount = static_cast< uint32_t >( extensionCount );

		return true;
	}

	bool VulkanApp::createInstance()
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
		{
			setError( VKAPP_ERROR_CREATE_INSTANCE );
			return false;
		}

		// Query available extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

		vulkan().extensions.resize( static_cast< size_t >( extensionCount ) );
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, vulkan().extensions.data() );

		stprintf( "[Vulkan]Available Extensions:\n" );

		for ( const auto &extension : vulkan().extensions )
			stprintf( "\t%s\n", extension.extensionName );

		stprintf( "\n" );

		return true;
	}

#if VULKAN_VALIDATION_LAYERS
	bool VulkanApp::setupDebugCallback()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		if ( CreateDebugUtilsMessengerEXT( vulkan().instance, &createInfo, nullptr, &vulkan().debugCallback ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_DEBUG_CALLBACK_CREATION );
			return false;
		}

		return true;
	}
#endif

	bool VulkanApp::createSurface()
	{
		if ( SDL_Vulkan_CreateSurface( GetVkRenderer_Internal().GetWindow(), vulkan().instance, &vulkan().surface ) != SDL_TRUE )
		{
			setError( VKAPP_ERROR_FAILED_SURFACE_CREATION );
			return false;
		}

		return true;
	}

	bool VulkanApp::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices( vulkan().instance, &deviceCount, nullptr );

		if ( deviceCount == 0 )
		{
			setError( VKAPP_ERROR_NO_PHYSICAL_DEVICE );
			return false;
		}

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

			return iScore;
		};

		// Ordered map that automatically sorts device candidates by increasing score
		std::multimap< uint32_t, VkPhysicalDevice > candidates;

		for ( auto &device : devices )
		{
			if ( !isDeviceSuitable( device ) )
				continue;

			uint32_t score = rateSuitability( device );
			candidates.insert( std::make_pair( score, device ) );
		}

		if ( !candidates.empty() && candidates.rbegin()->first > 0 )
			vulkan().physicalDevice = candidates.rbegin()->second;

		if ( vulkan().physicalDevice == VK_NULL_HANDLE )
		{
			setError( VKAPP_ERROR_NO_PHYSICAL_DEVICE );
			return false;
		}

		return true;
	}

	bool VulkanApp::isDeviceSuitable( VkPhysicalDevice &device )
	{
		auto checkDeviceExtensionSupport = [ & ]()
		{
			uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

			std::vector< VkExtensionProperties > availableExtensions( static_cast< size_t >( extensionCount ) );
			vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

			std::set< string > requiredExtensions( this->vulkan().deviceExtensions.begin(), this->vulkan().deviceExtensions.end() );

			for ( const auto &extension : availableExtensions )
				requiredExtensions.erase( extension.extensionName );

			return requiredExtensions.empty();
		};

		auto checkSwapChainSupport = [ & ]( std::shared_ptr< SwapChainSupportDetails > swapChainSupport )
		{
			return ( !swapChainSupport->formats.empty() && !swapChainSupport->presentModes.empty() );
		};


		return ( findQueueFamilies( device ).isComplete() && checkDeviceExtensionSupport() && checkSwapChainSupport( querySwapChainSupport( device ) ) ) ;
	}

	bool VulkanApp::createLogicalDevice()
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
		{
			setError( VKAPP_ERROR_LOGICAL_DEVICE_CREATION );
			return false;
		}

		vkGetDeviceQueue( vulkan().device, indices.graphicsFamily, 0, &vulkan().graphicsQueue );
		vkGetDeviceQueue( vulkan().device, indices.presentFamily, 0, &vulkan().presentQueue );

		return true;
	}

	bool VulkanApp::createSwapChain()
	{
		auto swapChainSupport = querySwapChainSupport( vulkan().physicalDevice );

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport->formats );
		VkPresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport->presentModes );
		VkExtent2D extent = chooseSwapExtent( swapChainSupport->capabilities );

		uint32_t imageCount = swapChainSupport->capabilities.maxImageCount > 0 ?
			std::min( swapChainSupport->capabilities.minImageCount + 1, swapChainSupport->capabilities.maxImageCount ) :
			swapChainSupport->capabilities.minImageCount + 1;

		VkSwapchainCreateInfoKHR createInfo = {};
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
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if ( vkCreateSwapchainKHR( vulkan().device, &createInfo, nullptr, &vulkan().swapChain ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_SWAP_CHAIN_CREATION );
			return false;
		}

		vkGetSwapchainImagesKHR( vulkan().device, vulkan().swapChain, &imageCount, nullptr );
		vulkan().swapChainImages.resize( imageCount, VK_NULL_HANDLE );
		vkGetSwapchainImagesKHR( vulkan().device, vulkan().swapChain, &imageCount, vulkan().swapChainImages.data() );

		vulkan().swapChainImageFormat = surfaceFormat.format;
		vulkan().swapChainExtent = extent;

		return true;
	}

	bool VulkanApp::createImageViews()
	{
		vulkan().swapChainImageViews.resize( vulkan().swapChainImages.size(), VK_NULL_HANDLE );

		for ( size_t i = 0; i < vulkan().swapChainImages.size(); i++ )
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = vulkan().swapChainImages[ i ];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = vulkan().swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if ( vkCreateImageView( vulkan().device, &createInfo, nullptr, &vulkan().swapChainImageViews[ i ] ) != VK_SUCCESS )
			{
				setError( VKAPP_ERROR_IMAGE_VIEW_CREATION );
				return false;
			}
		}

		return true;
	}

	bool VulkanApp::createRenderPass()
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

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if ( vkCreateRenderPass( vulkan().device, &renderPassInfo, nullptr, &vulkan().renderPass ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_RENDER_PASS_CREATION );
			return false;
		}

		return true;
	}

	bool VulkanApp::createGraphicsPipeline()
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
			{
				setError( VKAPP_ERROR_GRAPIHCS_PIPELINE_CREATION );
				return false;
			}
		}

		VkShaderModule shaderModules[ SHADER_COUNT ] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

		for ( size_t i = 0; i < SHADER_COUNT; i++ )
		{
			shaderModules[ i ] = createShaderModule( shaderCode[ i ] );

			if ( shaderModules[ i ] == VK_NULL_HANDLE )
			{
				setError( VKAPP_ERROR_GRAPIHCS_PIPELINE_CREATION );
				return false;
			}
		}

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shaderModules[ VERTEX_SHADER ];
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shaderModules[ FRAGMENT_SHADER ];
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = ( float )vulkan().swapChainExtent.width;
		viewport.height = ( float )vulkan().swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = vulkan().swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // TODO: Revisit this
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // TODO: Revisit this
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optioanl
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		/*Alpha Blending*/
		/*colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
		colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
		colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
		colorBlending.blendConstants[ 3 ] = 0.0f; // Optional

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		auto destroyShaderModules = [ & ]()
		{
			for ( auto &shaderModule : shaderModules )
				vkDestroyShaderModule( vulkan().device, shaderModule, nullptr );
		};

		if ( vkCreatePipelineLayout( vulkan().device, &pipelineLayoutInfo, nullptr, &vulkan().pipelineLayout ) != VK_SUCCESS )
		{
			destroyShaderModules();
			setError( VKAPP_ERROR_GRAPIHCS_PIPELINE_CREATION );

			return false;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = vulkan().pipelineLayout;
		pipelineInfo.renderPass = vulkan().renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if ( vkCreateGraphicsPipelines( vulkan().device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vulkan().graphicsPipeline ) != VK_SUCCESS )
		{
			destroyShaderModules();
			setError( VKAPP_ERROR_GRAPIHCS_PIPELINE_CREATION );

			return false;
		}

		destroyShaderModules();

		return true;
	}

	bool VulkanApp::createFramebuffers()
	{
		vulkan().swapChainFramebuffers.resize( vulkan().swapChainImageViews.size(), VK_NULL_HANDLE );

		for ( size_t i = 0; i < vulkan().swapChainFramebuffers.size(); ++i )
		{
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = vulkan().renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &vulkan().swapChainImageViews[ i ];
			framebufferInfo.width = vulkan().swapChainExtent.width;
			framebufferInfo.height = vulkan().swapChainExtent.height;
			framebufferInfo.layers = 1;

			if ( vkCreateFramebuffer( vulkan().device, &framebufferInfo, nullptr, &vulkan().swapChainFramebuffers[ i ] ) != VK_SUCCESS )
			{
				setError( VKAPP_ERROR_FRAMEBUFFER_CREATION );
				return false;
			}
		}

		return true;
	}

	bool VulkanApp::createCommandPool()
	{
		auto queueFamilyIndices = findQueueFamilies( vulkan().physicalDevice );

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = 0; // Optional

		if ( vkCreateCommandPool( vulkan().device, &poolInfo, nullptr, &vulkan().commandPool ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_COMMAND_POOL_CREATION );
			return false;
		}

		return true;
	}

	bool VulkanApp::createCommandBuffers()
	{
		vulkan().commandBuffers.resize( vulkan().swapChainFramebuffers.size(), VK_NULL_HANDLE );

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vulkan().commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast< uint32_t >( vulkan().commandBuffers.size() );

		if ( vkAllocateCommandBuffers( vulkan().device, &allocInfo, vulkan().commandBuffers.data() ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_COMMAND_BUFFER_CREATION );
			return false;
		}

		for ( size_t i = 0; i < vulkan().commandBuffers.size(); ++i )
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if ( vkBeginCommandBuffer( vulkan().commandBuffers[ i ], &beginInfo ) != VK_SUCCESS )
			{
				setError( VKAPP_ERROR_COMMAND_BUFFER_CREATION );
				return false;
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = vulkan().renderPass;
			renderPassInfo.framebuffer = vulkan().swapChainFramebuffers[ i ];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = vulkan().swapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.5f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass( vulkan().commandBuffers[ i ], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
				vkCmdBindPipeline( vulkan().commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan().graphicsPipeline );
				vkCmdDraw( vulkan().commandBuffers[ i ], 3, 1, 0, 0 );
			vkCmdEndRenderPass( vulkan().commandBuffers[ i ] );

			if ( vkEndCommandBuffer( vulkan().commandBuffers[ i ] ) != VK_SUCCESS )
			{
				setError( VKAPP_ERROR_COMMAND_BUFFER_CREATION );
				return false;
			}
		}

		return true;
	}

	bool VulkanApp::createSyncObjects()
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
			if ( vkCreateSemaphore( vulkan().device, &semaphoreInfo, nullptr, &vulkan().imageAvailableSemaphores[ i ] ) != VK_SUCCESS ||
				vkCreateSemaphore( vulkan().device, &semaphoreInfo, nullptr, &vulkan().renderFinishedSemaphores[ i ] ) != VK_SUCCESS ||
				vkCreateFence( vulkan().device, &fenceInfo, nullptr, &vulkan().inFlightFences[ i ] ) != VK_SUCCESS )
			{
				setError( VKAPP_ERROR_SYNC_OBJECT_CREATION );
				return false;
			}
		}

		return true;
	}

	bool VulkanApp::recreateSwapChain()
	{
		vkDeviceWaitIdle( vulkan().device );

		cleanupSwapChain();

		return createSwapChain() && createImageViews() && createRenderPass() && createGraphicsPipeline() && createFramebuffers() && createCommandBuffers();
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
	VkPresentModeKHR VulkanApp::chooseSwapPresentMode( const std::vector< VkPresentModeKHR > availablePresentModes )
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
			return VK_NULL_HANDLE;

		return shaderModule;
	}
}