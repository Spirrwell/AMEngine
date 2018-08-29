#ifndef VULKAN_HELPERS_HPP
#define VULKAN_HELPERS_HPP

#include "vulkan/vulkan.hpp"
#include "string.hpp"

#include <vector>

class RendererVulkan;

#define VULKAN_VALIDATION_LAYERS 1

namespace vkApp
{
	class VulkanApp
	{
		friend class RendererVulkan;

		static const char *s_pszErrors[];

		enum vkAppError
		{
			VKAPP_ERROR_NONE,
			VKAPP_ERROR_REQUIRED_EXTENSIONS,
			VKAPP_ERROR_CREATE_INSTANCE,
#if VULKAN_VALIDATION_LAYERS
			VKAPP_ERROR_VALIDATION_LAYERS,
			VKAPP_ERROR_DEBUG_CALLBACK_CREATION
#endif
		};

		// Our Vulkan variables\info will be stored here
		struct vulkanContainer
		{
			std::vector< VkExtensionProperties > extensions;
			VkInstance instance = VK_NULL_HANDLE;

#if VULKAN_VALIDATION_LAYERS
			VkDebugUtilsMessengerEXT debugCallback = VK_NULL_HANDLE;

			const std::vector< const char * > validationLayers =
			{ 
				"VK_LAYER_LUNARG_standard_validation"
			};
#endif
		};

	public:
		VulkanApp();
		virtual ~VulkanApp();

	protected:
		bool initVulkan();
		void cleanup();
		
		vulkanContainer m_Vulkan;

	private:
		vkAppError m_Error = VKAPP_ERROR_NONE;

		// Required extensions loaded from SDL
		const char **m_ppszReqExtensionNames = nullptr;
		uint32_t m_iReqExtCount = 0;

		inline void setError( vkAppError error ) { m_Error = error; }
		inline void printError() { stprintf( "[Vulkan]%s\n", s_pszErrors[ m_Error ] ); }

#if VULKAN_VALIDATION_LAYERS
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData );
		static VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback );
		static void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator );
		bool checkValidationLayerSupport();
#endif

		bool loadExtensionsFromSDL();
		bool createInstance();

#if VULKAN_VALIDATION_LAYERS
		bool setupDebugCallback();
#endif

		std::vector< const char * > getRequiredExtensions();
		
	};
}

#endif // VULKAN_HELPERS_HPP