#include "vulkan_interface.hpp"
#include "vulkan_helpers.hpp"
#include "renderer_vulkan.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

namespace vkApp
{
	CVulkanInterface::CVulkanInterface() : m_vkApp( GetVkRenderer_Internal().VulkanApp() ) {}
	VulkanApp &CVulkanInterface::VulkanApp() { return m_vkApp; }
	VulkanContext &CVulkanInterface::vulkan() { return m_vkApp.vulkan(); }
}