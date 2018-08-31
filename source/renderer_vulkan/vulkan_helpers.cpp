#include "vulkan_helpers.hpp"
#include "renderer_vulkan.hpp"
#include "engine/config.hpp"

#include "SDL_vulkan.h"

#include <algorithm>
#include <memory>
#include <map>
#include <set>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

namespace vkApp
{
	const char *VulkanApp::s_pszErrors[] =
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
		"Failed to create swap chain."
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

		return true;
	}

	void VulkanApp::cleanup()
	{
		if ( vulkan().swapChain != VK_NULL_HANDLE )
		{
			vkDestroySwapchainKHR( vulkan().device, vulkan().swapChain, nullptr );
			vulkan().swapChain = VK_NULL_HANDLE;
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
			createInfo.queueFamilyIndexCount = static_cast< uint32_t >( sizeof( queueFamilyIndices ) );
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
		vulkan().swapChainImages.resize( imageCount );
		vkGetSwapchainImagesKHR( vulkan().device, vulkan().swapChain, &imageCount, vulkan().swapChainImages.data() );

		vulkan().swapChainImageFormat = surfaceFormat.format;
		vulkan().swapChainExtent = extent;

		return true;
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
}