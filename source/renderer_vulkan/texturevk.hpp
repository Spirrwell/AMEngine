#ifndef TEXTUREVK_HPP
#define TEXTUREVK_HPP

#include "vulkan/vulkan.hpp"
#include "vulkan_interface.hpp"
#include "string.hpp"

#include <stdint.h>

class TextureVK : vkApp::CVulkanInterface
{
public:
	virtual ~TextureVK();
	void Load( const string &path );
	void Shutdown();

	const VkImageView &ImageView() { return m_vkTextureImageView; }
	const VkSampler &Sampler() { return m_vkTextureSampler; }

private:
	VkImage m_vkTextureImage = VK_NULL_HANDLE;
	VkDeviceMemory m_vkTextureImageMemory = VK_NULL_HANDLE;
	VkImageView m_vkTextureImageView = VK_NULL_HANDLE;
	VkSampler m_vkTextureSampler = VK_NULL_HANDLE;

	uint32_t m_nMipLevels = 0;
};

#endif // TEXTUREVK_HPP