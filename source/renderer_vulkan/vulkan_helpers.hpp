#ifndef VULKAN_HELPERS_HPP
#define VULKAN_HELPERS_HPP

#include "sdl_core/sdleventlistener.hpp"
#include "vulkan/vulkan.hpp"
#include "string.hpp"

#include <vector>
#include <memory>

class RendererVulkan;

#define VULKAN_VALIDATION_LAYERS 1

namespace vkApp
{
	class VulkanApp : public SDLEventListener
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
			VKAPP_ERROR_DEBUG_CALLBACK_CREATION,
#endif
			VKAPP_ERROR_FAILED_SURFACE_CREATION,
			VKAPP_ERROR_NO_PHYSICAL_DEVICE,
			VKAPP_ERROR_LOGICAL_DEVICE_CREATION,
			VKAPP_ERROR_SWAP_CHAIN_CREATION,
			VKAPP_ERROR_IMAGE_VIEW_CREATION,
			VKAPP_ERROR_RENDER_PASS_CREATION,
			VKAPP_ERROR_GRAPIHCS_PIPELINE_CREATION,
			VKAPP_ERROR_FRAMEBUFFER_CREATION,
			VKAPP_ERROR_COMMAND_POOL_CREATION,
			VKAPP_ERROR_COMMAND_BUFFER_CREATION,
			VKAPP_ERROR_SYNC_OBJECT_CREATION,
			
			VKAPP_ERROR_COUNT
		};

		struct QueueFamilyIndex
		{
			QueueFamilyIndex() { index = 0; }

			explicit operator bool() { return bSet; }
			operator uint32_t() { return index; }
			QueueFamilyIndex &operator=( const uint32_t &index ) { this->index = index; bSet = true; return *this; }

		private:
			uint32_t index;
			bool bSet = false;
		};

		struct QueueFamilyIndices
		{
			QueueFamilyIndex graphicsFamily;
			QueueFamilyIndex presentFamily;

			bool isComplete()
			{
				return ( ( bool )graphicsFamily && ( bool )presentFamily );
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector< VkSurfaceFormatKHR > formats;
			std::vector< VkPresentModeKHR > presentModes;
		};

		// Our Vulkan variables\info will be stored here
		struct vulkanContainer
		{
			std::vector< VkExtensionProperties > extensions;
			const std::vector< const char * > deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			VkInstance instance = VK_NULL_HANDLE;

#if VULKAN_VALIDATION_LAYERS
			VkDebugUtilsMessengerEXT debugCallback = VK_NULL_HANDLE;

			const std::vector< const char * > validationLayers =
			{ 
				"VK_LAYER_LUNARG_standard_validation"
			};
#endif
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;
			VkQueue graphicsQueue = VK_NULL_HANDLE;
			VkQueue presentQueue = VK_NULL_HANDLE;
			VkSwapchainKHR swapChain = VK_NULL_HANDLE;
			std::vector< VkImage > swapChainImages;
			VkFormat swapChainImageFormat;
			VkExtent2D swapChainExtent;
			std::vector< VkImageView > swapChainImageViews;
			VkRenderPass renderPass = VK_NULL_HANDLE;
			VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
			VkPipeline graphicsPipeline = VK_NULL_HANDLE;
			std::vector< VkFramebuffer > swapChainFramebuffers;
			VkCommandPool commandPool = VK_NULL_HANDLE;
			std::vector< VkCommandBuffer > commandBuffers;
			std::vector< VkSemaphore > imageAvailableSemaphores;
			std::vector< VkSemaphore > renderFinishedSemaphores;
			std::vector< VkFence > inFlightFences;

			bool bFrameBufferResized = false;
			bool bMinimized = false;
		};

	public:
		VulkanApp();
		virtual ~VulkanApp();

		const vulkanContainer &vulkan() const { return m_Vulkan; }

		static constexpr const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	protected:
		vulkanContainer &vulkan() { return m_Vulkan; }

		bool initVulkan();
		void cleanup();
		void cleanupSwapChain();

		void drawFrame();

	private:
		vulkanContainer m_Vulkan;

		vkAppError m_Error = VKAPP_ERROR_NONE;
		QueueFamilyIndices m_QueueFamilyIndices;

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

		void ProcessEvent( const SDL_Event &event ) override;

		bool loadExtensionsFromSDL();
		bool createInstance();

#if VULKAN_VALIDATION_LAYERS
		bool setupDebugCallback();
#endif
		bool createSurface();
		bool pickPhysicalDevice();
		bool isDeviceSuitable( VkPhysicalDevice &device );
		bool createLogicalDevice();
		bool createSwapChain();
		bool createImageViews();
		bool createRenderPass();
		bool createGraphicsPipeline();
		bool createFramebuffers();
		bool createCommandPool();
		bool createCommandBuffers();
		bool createSyncObjects();

		bool recreateSwapChain();

		std::vector< const char * > getRequiredExtensions();
		QueueFamilyIndices findQueueFamilies( VkPhysicalDevice &device );
		std::shared_ptr< SwapChainSupportDetails > querySwapChainSupport( VkPhysicalDevice &device );
		VkSurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< VkSurfaceFormatKHR > &availableFormats );
		VkPresentModeKHR chooseSwapPresentMode( const std::vector< VkPresentModeKHR > availablePresentModes );
		VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR &capabilities );
		VkShaderModule createShaderModule( const std::vector< std::byte > &code );
	};
}

#endif // VULKAN_HELPERS_HPP