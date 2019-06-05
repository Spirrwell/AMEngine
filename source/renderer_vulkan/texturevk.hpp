#ifndef TEXTUREVK_HPP
#define TEXTUREVK_HPP

#include <filesystem>

#include "vulkan/vulkan.hpp"
#include "vulkan_interface.hpp"
#include "string.hpp"
#include "ktxvulkan.h"

#include <stdint.h>
#include <array>

class TextureVK : vkApp::CVulkanInterface
{
public:
	virtual ~TextureVK();
	void Load( const std::filesystem::path &path );
	void Load( const std::array< string, 6 > &faces );
	void LoadRGBA( const unsigned char *pPixels, size_t width, size_t height, bool bGenMipMaps = false );
	void LoadKtx( const std::filesystem::path &ktxFile );

	void Shutdown();

	const VkImageView &ImageView() { return m_vkTextureImageView; }
	const VkSampler &Sampler() { return m_vkTextureSampler; }

private:
	VkImage m_vkTextureImage = VK_NULL_HANDLE;
	VkDeviceMemory m_vkTextureImageMemory = VK_NULL_HANDLE;
	VkImageView m_vkTextureImageView = VK_NULL_HANDLE;
	VkSampler m_vkTextureSampler = VK_NULL_HANDLE;

	uint32_t m_nMipLevels = 1;
	ktxVulkanTexture m_ktxVulkanTexture = {};
};

#endif // TEXTUREVK_HPP