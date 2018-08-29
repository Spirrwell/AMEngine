#include "vulkan_helpers.hpp"
#include "renderer_vulkan.hpp"
#include "SDL_vulkan.h"

#include <memory>

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
		"Failed to set up debug callback."
#endif
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
		{
			printError();
			return false;
		}
#if VULKAN_VALIDATION_LAYERS
		if ( !checkValidationLayerSupport() )
		{
			printError();
			return false;
		}
#endif
		if ( !createInstance() )
		{
			printError();
			return false;
		}

#if VULKAN_VALIDATION_LAYERS
		if ( !setupDebugCallback() )
		{
			printError();
			return false;
		}
#endif

		return true;
	}

	void VulkanApp::cleanup()
	{
#if VULKAN_VALIDATION_LAYERS
		if ( m_Vulkan.debugCallback != VK_NULL_HANDLE )
		{
			DestroyDebugUtilsMessengerEXT( m_Vulkan.instance, m_Vulkan.debugCallback, nullptr );
			m_Vulkan.debugCallback = VK_NULL_HANDLE;
		}
#endif

		if ( m_Vulkan.instance != VK_NULL_HANDLE )
		{
			vkDestroyInstance( m_Vulkan.instance, nullptr );
			m_Vulkan.instance = VK_NULL_HANDLE;
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

		for ( auto &layerName : m_Vulkan.validationLayers )
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
		createInfo.enabledLayerCount = static_cast< uint32_t >( m_Vulkan.validationLayers.size() );
		createInfo.ppEnabledLayerNames = m_Vulkan.validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		if ( vkCreateInstance( &createInfo, nullptr, &m_Vulkan.instance ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_CREATE_INSTANCE );
			return false;
		}

		// Query available extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

		m_Vulkan.extensions.resize( static_cast< size_t >( extensionCount ) );
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, m_Vulkan.extensions.data() );

		stprintf( "[Vulkan]Available Extensions:\n" );

		for ( const auto &extension : m_Vulkan.extensions )
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

		if ( CreateDebugUtilsMessengerEXT( m_Vulkan.instance, &createInfo, nullptr, &m_Vulkan.debugCallback ) != VK_SUCCESS )
		{
			setError( VKAPP_ERROR_DEBUG_CALLBACK_CREATION );
			return false;
		}

		return true;
	}
#endif

	std::vector< const char * > VulkanApp::getRequiredExtensions()
	{
		std::vector< const char * > extensions( m_ppszReqExtensionNames, m_ppszReqExtensionNames + m_iReqExtCount );

#if VULKAN_VALIDATION_LAYERS
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

		return extensions;
	}
}