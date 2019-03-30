#ifndef VULKAN_INTERFACE_HPP
#define VULKAN_INTERFACE_HPP

namespace vkApp
{
	class VulkanApp;
	struct VulkanContext;

	class CVulkanInterface
	{
	public:
		CVulkanInterface();

		vkApp::VulkanApp &VulkanApp();
		VulkanContext &vulkan();

	private:
		vkApp::VulkanApp &m_vkApp;
	};
}

#endif // VULKAN_INTERFACE_HPP