#ifndef VULKAN_HELPERS_HPP
#define VULKAN_HELPERS_HPP

#include "sdl_core/sdleventlistener.hpp"
#include "vulkan/vulkan.hpp"
#include "string.hpp"
#include "texturevk.hpp"

#include <vector>
#include <memory>

class RendererVulkan;

#define VULKAN_VALIDATION_LAYERS 1

namespace vkApp
{
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
		VkDescriptorSetLayout descripterSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector< VkDescriptorSet > descriptorSets;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline graphicsPipeline = VK_NULL_HANDLE;
		std::vector< VkFramebuffer > swapChainFramebuffers;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		TextureVK texture;
		VkImage depthImage = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;

		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

		std::vector< VkBuffer > uniformBuffers;
		std::vector< VkDeviceMemory > uniformBuffersMemory;

		std::vector< VkCommandBuffer > commandBuffers;
		std::vector< VkSemaphore > imageAvailableSemaphores;
		std::vector< VkSemaphore > renderFinishedSemaphores;
		std::vector< VkFence > inFlightFences;

		bool bFrameBufferResized = false;
		bool bMinimized = false;
	};

	class VulkanApp : public SDLEventListener
	{
		friend class ::RendererVulkan;
		friend class CVulkanInterface;

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

	public:
		VulkanApp();
		virtual ~VulkanApp();

		vulkanContainer &vulkan() { return m_Vulkan; }

		static constexpr const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	protected:

		bool initVulkan();
		void cleanup();
		void cleanupSwapChain();

		void drawFrame();
		void updateUniformBuffer( const uint32_t &currentImage );

	private:
		vulkanContainer m_Vulkan;
		QueueFamilyIndices m_QueueFamilyIndices;

		// Required extensions loaded from SDL
		const char **m_ppszReqExtensionNames = nullptr;
		uint32_t m_iReqExtCount = 0;

#if VULKAN_VALIDATION_LAYERS
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData );
		static VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback );
		static void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator );
		void checkValidationLayerSupport();
#endif

		void ProcessEvent( const SDL_Event &event ) override;

		void loadExtensionsFromSDL();
		void createInstance();

#if VULKAN_VALIDATION_LAYERS
		void setupDebugCallback();
#endif
		void createSurface();
		void pickPhysicalDevice();
		bool isDeviceSuitable( VkPhysicalDevice &device );
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createDescriptorSetLayout(); // UBOs
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createDepthResources();

		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffer();
		void createDescriptorPool();
		void createDescriptorSets();

		void createCommandBuffers();
		void createSyncObjects();

	public:
		void createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory );
		void createImage( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory );
		VkImageView createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags );
		void copyBuffer( const VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize size );
		void copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height );
		
	private:
		void recreateSwapChain();

	public:
		std::vector< const char * > getRequiredExtensions();
		QueueFamilyIndices findQueueFamilies( VkPhysicalDevice &device );
		uint32_t findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );
		std::shared_ptr< SwapChainSupportDetails > querySwapChainSupport( VkPhysicalDevice &device );
		VkSurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< VkSurfaceFormatKHR > &availableFormats );
		VkPresentModeKHR chooseSwapPresentMode( const std::vector< VkPresentModeKHR > availablePresentModes );
		VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR &capabilities );
		VkShaderModule createShaderModule( const std::vector< std::byte > &code );
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands( VkCommandBuffer commandBuffer );
		void transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout );
		VkFormat findSupportedFormat( const std::vector< VkFormat > &candidates, VkImageTiling tiling, VkFormatFeatureFlags features );
		VkFormat findDepthFormat();
		bool hasStencilComponent( VkFormat format );
	};
}

#endif // VULKAN_HELPERS_HPP