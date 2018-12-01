#include "texturevk.hpp"
#include "vulkan_helpers.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

TextureVK::~TextureVK()
{
	Shutdown();
}

void TextureVK::Load( const string &path )
{
	// Create Texture Image
	int texWidth = 0, texHeight = 0, numComponents = 0;
	stbi_uc *pPixels = stbi_load( path.c_str(), &texWidth, &texHeight, &numComponents, STBI_rgb_alpha );

	if ( !pPixels )
		throw std::runtime_error( "[Vulkan]Failed to load texture image!" );

	VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	VulkanApp().createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;
	vkMapMemory( vulkan().device, stagingBufferMemory, 0, imageSize, 0, &pData );
		std::memcpy( pData, pPixels, static_cast< size_t >( imageSize ) );
	vkUnmapMemory( vulkan().device, stagingBufferMemory );

	stbi_image_free( pPixels );
	VulkanApp().createImage( static_cast< uint32_t >( texWidth ),
			static_cast< uint32_t >( texHeight ),
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vkTextureImage,
			m_vkTextureImageMemory );

	VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
	VulkanApp().copyBufferToImage( stagingBuffer, m_vkTextureImage, static_cast< uint32_t >( texWidth ), static_cast< uint32_t >( texHeight ) );
	VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

	vkDestroyBuffer( vulkan().device, stagingBuffer, nullptr );
	vkFreeMemory( vulkan().device, stagingBufferMemory, nullptr );

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

	// Create Texture Sampler
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if ( vkCreateSampler( vulkan().device, &samplerInfo, nullptr, &m_vkTextureSampler ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );
}

void TextureVK::Shutdown()
{
	if ( m_vkTextureSampler != VK_NULL_HANDLE )
	{
		vkDestroySampler( vulkan().device, m_vkTextureSampler, nullptr );
		m_vkTextureSampler = VK_NULL_HANDLE;
	}

	if ( m_vkTextureImageView != VK_NULL_HANDLE )
	{
		vkDestroyImageView( vulkan().device, m_vkTextureImageView, nullptr );
		m_vkTextureImageView = VK_NULL_HANDLE;
	}

	if ( m_vkTextureImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( vulkan().device, m_vkTextureImage, nullptr );
		m_vkTextureImage = VK_NULL_HANDLE;
	}

	if ( m_vkTextureImageMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( vulkan().device, m_vkTextureImageMemory, nullptr );
		m_vkTextureImageMemory = VK_NULL_HANDLE;
	}
}