#ifndef VULKAN_INTERFACE_HPP
#define VULKAN_INTERFACE_HPP

namespace vkApp
{
	class VulkanApp;
	struct vulkanContainer;

	class CVulkanInterface
	{
	public:
		CVulkanInterface();

		vkApp::VulkanApp &VulkanApp();
		vulkanContainer &vulkan();

	private:
		vkApp::VulkanApp &m_vkApp;
	};
}

#endif // VULKAN_INTERFACE_HPP