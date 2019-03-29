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
	void Load( const string &path );
	void Load( const std::array< string, 6 > &faces );
	void LoadKtx( const std::filesystem::path &ktxFile );
	void Shutdown();

	const vk::ImageView &ImageView() { return m_vkTextureImageView; }
	const vk::Sampler &Sampler() { return m_vkTextureSampler; }

private:
	vk::Image m_vkTextureImage;
	vk::DeviceMemory m_vkTextureImageMemory;
	vk::ImageView m_vkTextureImageView;
	vk::Sampler m_vkTextureSampler;

	uint32_t m_nMipLevels = 1;
	ktxVulkanTexture m_ktxVulkanTexture = {};
};

#endif // TEXTUREVK_HPP