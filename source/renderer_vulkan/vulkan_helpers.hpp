#ifndef VULKAN_HELPERS_HPP
#define VULKAN_HELPERS_HPP

#include "sdl_core/sdleventlistener.hpp"
#include "vulkan/vulkan.hpp"
#include "string.hpp"
#include "texturevk.hpp"

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

class RendererVulkan;
class ShaderVK;
class MeshVK;
class MaterialVK;
class MaterialDiffuseOnly;
class ModelVK;

#define VULKAN_VALIDATION_LAYERS 0

namespace vkApp
{
	class VulkanThreadPool
	{
	public:
		
		void InitThreads()
		{
			m_pThreads.resize( m_NumThreads / 2, nullptr );

			for ( auto it = m_pThreads.begin(); it != m_pThreads.end(); ++it )
			{
				*it = new CBThread;
				( *it )->InitThread();
			}
		}

		void TerminateThreads()
		{
			for ( auto *pThread : m_pThreads )
			{
				pThread->Terminate();
				delete pThread;
			}

			m_pThreads.clear();
			ClearJobs();
		}

		void WaitIdle()
		{
			for ( auto *pThread : m_pThreads )
				while ( !pThread->IsIdle() ){}
		}

		void ClearJobs()
		{
			for ( auto *pThread : m_pThreads )
				pThread->ClearJobs();

			m_Jobs.clear();
		}

		class CBJob
		{
		public:
			CBJob( uint32_t imageIndex, size_t meshIndex )
			{
				m_iImageIndex = imageIndex;
				m_iMeshIndex = meshIndex;
			}

			void Execute();
			const vk::CommandBuffer *GetResult() const { return m_pResult; }

		private:
			uint32_t m_iImageIndex;
			size_t m_iMeshIndex;
			const vk::CommandBuffer *m_pResult = nullptr;
		};

		class CBThread
		{
		public:
			
			void InitThread()
			{
				m_pThread = new std::thread( &CBThread::Run, this );
				m_pThread->detach();
			}

			void DoJobs()
			{
				if ( m_pJobs.size() > 0 )
					m_bIdle = false;
			}

			void Terminate()
			{
				m_bTerminate = true;

				if ( m_pThread )
				{
					if ( m_pThread->joinable() )
						m_pThread->join();
					else
						while ( !IsIdle() ){}

					delete m_pThread;
				}
			}

			// Be sure not to add jobs while executing them
			void AddJob( CBJob *pJob )
			{
				m_pJobs.push_back( pJob );
			}

			// Be sure not to clear jobs while executing them
			void ClearJobs()
			{
				m_pJobs.clear();
			}

			const std::atomic_bool &IsIdle() const { return m_bIdle; }

		protected:
			void Run()
			{
				using namespace std::literals::chrono_literals;
				while ( !m_bTerminate )
				{
					if ( !IsIdle() )
					{
						for ( auto *pJob : m_pJobs )
							pJob->Execute();

						m_bIdle = true;
					}
					else
						std::this_thread::yield();
				}
			}

		private:
			std::thread *m_pThread = nullptr;
			std::atomic_bool m_bIdle = true;
			std::atomic_bool m_bTerminate = false;
			std::vector< CBJob* > m_pJobs;
		};

		void AddJob( const CBJob &job )
		{
			m_Jobs.push_back( job );
		}

		const std::vector< CBJob > &GetJobs() const { return m_Jobs; }

		// Assigns jobs to threads
		void AssignJobs()
		{
			if ( m_pThreads.size() == 0 )
				return;

			auto threadIt = m_pThreads.begin();
			auto AssignJob = [ &, this ]( CBJob *job )
			{
				( *threadIt )->AddJob( job );
				if ( ++threadIt == m_pThreads.end() )
					threadIt = m_pThreads.begin();
			};

			for ( CBJob &job : m_Jobs )
				AssignJob( &job );
		}

		void ExecuteJobs()
		{
			for ( auto *pThread : m_pThreads )
				pThread->DoJobs();
		}

	private:

		unsigned int m_NumThreads = std::thread::hardware_concurrency();
		std::vector< CBThread* > m_pThreads;
		std::vector< CBJob > m_Jobs;
	};

	// Our Vulkan variables\info will be stored here
	struct VulkanContext
	{
		std::vector< vk::ExtensionProperties > extensions;
		const std::vector< const char * > deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vk::Instance instance;

#if VULKAN_VALIDATION_LAYERS
		vk::DebugUtilsMessengerEXT debugCallback;

		const std::vector< const char * > validationLayers =
		{ 
			"VK_LAYER_LUNARG_standard_validation"
		};
#endif
		vk::SurfaceKHR surface;
		//VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		vk::SwapchainKHR swapChain;
		std::vector< vk::Image > swapChainImages;
		vk::Format swapChainImageFormat;
		vk::Extent2D swapChainExtent;
		std::vector< vk::ImageView > swapChainImageViews;
		vk::RenderPass renderPass;
		std::vector< vk::Framebuffer > swapChainFramebuffers;
		vk::CommandPool commandPool;
		vk::Image depthImage;
		vk::DeviceMemory depthImageMemory;
		vk::ImageView depthImageView;

		std::vector< vk::Buffer > uniformBuffers;
		std::vector< vk::DeviceMemory > uniformBuffersMemory;

		std::vector< vk::CommandBuffer > commandBuffers;
		std::vector< vk::Semaphore > imageAvailableSemaphores;
		std::vector< vk::Semaphore > renderFinishedSemaphores;
		std::vector< vk::Fence > inFlightFences;

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
			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector< vk::SurfaceFormatKHR > formats;
			std::vector< vk::PresentModeKHR > presentModes;
		};

	public:
		VulkanApp();
		virtual ~VulkanApp();

		VulkanContext &vulkan() { return m_Vulkan; }

		void AddShader( ShaderVK *pShader ) { m_pShaders.push_back( pShader ); }
		void RemoveShader( ShaderVK *pShader ) { for ( auto it = m_pShaders.begin(); it != m_pShaders.end(); ++it ) if ( *it == pShader ) { m_pShaders.erase( it ); return; } }

		void AddMesh( MeshVK &mesh ) { m_pMeshes.push_back( &mesh ); }
		void RemoveMesh( MeshVK *pMesh ) { for ( auto it = m_pMeshes.begin(); it != m_pMeshes.end(); ++it ) if ( *it == pMesh ) { m_pMeshes.erase( it ); return; } }

		const std::vector< ShaderVK* > &GetShaders() { return m_pShaders; }

		static constexpr const auto MAX_FRAMES_IN_FLIGHT = 2;

	protected:

		bool initVulkan();
		void cleanup();
		void cleanupSwapChain();

		void drawFrame();
		void updateUniformBuffer( const uint32_t &currentImage );

	private:
		VulkanContext m_Vulkan;
		QueueFamilyIndices m_QueueFamilyIndices;

		std::vector< ShaderVK* > m_pShaders;
	public:
		std::vector< MeshVK* > m_pMeshes;
	private:

		// Required extensions loaded from SDL
		const char **m_ppszReqExtensionNames = nullptr;
		uint32_t m_iReqExtCount = 0;

#if VULKAN_VALIDATION_LAYERS
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData );
		static vk::Result CreateDebugUtilsMessengerEXT( vk::Instance instance, const vk::DebugUtilsMessengerCreateInfoEXT *pCreateInfo, const vk::AllocationCallbacks *pAllocator, vk::DebugUtilsMessengerEXT *pCallback );
		static void DestroyDebugUtilsMessengerEXT( vk::Instance instance, vk::DebugUtilsMessengerEXT callback, const vk::AllocationCallbacks *pAllocator );
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
		bool isDeviceSuitable( vk::PhysicalDevice &device );
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createCommandPool();
		void createDepthResources();

		void createUniformBuffer();

		void allocateCommandBuffers();
		void recordCommandBuffer( const uint32_t &imageIndex );
		void recordCommandBuffers();
		void createSyncObjects();

	public:
		void createBuffer( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory );
		void createImage( uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory, bool bCubeMap = false );
		vk::ImageView createImageView( vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, vk::ImageViewType imageViewType );
		void copyBuffer( const vk::Buffer &srcBuffer, vk::Buffer &dstBuffer, vk::DeviceSize size );
		void copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t numComponents, bool bCubeMap = false );
		
	private:
		void recreateSwapChain();

	public:
		std::vector< const char * > getRequiredExtensions();
		QueueFamilyIndices findQueueFamilies( vk::PhysicalDevice &device );
		uint32_t findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags properties );
		std::shared_ptr< SwapChainSupportDetails > querySwapChainSupport( vk::PhysicalDevice &device );
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR > &availableFormats );
		vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR > &availablePresentModes );
		vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR &capabilities );
		vk::ShaderModule createShaderModule( const std::vector< std::byte > &code );
		vk::CommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands( vk::CommandBuffer &commandBuffer );
		void transitionImageLayout( vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels, bool bCubeMap = false );
		vk::Format findSupportedFormat( const std::vector< vk::Format > &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );
		vk::Format findDepthFormat();
		bool hasStencilComponent( vk::Format format );
		void generateMipMaps( vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, bool bCubeMap = false );

		//MeshVK *m_pTestMesh = nullptr;
		//MaterialVK *m_pTestMaterial = nullptr;

		ModelVK *m_pTestModel = nullptr;
		ModelVK *m_pTestModel2 = nullptr;
		MeshVK *m_pSkyboxTest = nullptr;
		MaterialVK *m_pTestMaterial = nullptr;
		std::vector< MeshVK* > m_pTestMeshes;
		MaterialVK *m_pSkyboxMaterial = nullptr;

		VulkanThreadPool m_ThreadPool;
	};
}

#endif // VULKAN_HELPERS_HPP