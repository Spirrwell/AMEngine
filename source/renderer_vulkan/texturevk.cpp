#include "texturevk.hpp"
#include "vulkan_helpers.hpp"
#include "amlib/memorywriter.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

TextureVK::~TextureVK()
{
	Shutdown();
}

void TextureVK::Load( const std::filesystem::path &path )
{
	if ( string extension = path.extension().string(); !extension.empty() )
	{
		std::transform( extension.begin(), extension.end(), extension.begin(), ::tolower );

		if ( extension == ".ktx" )
			return LoadKtx( path );
	}

	// Create Texture Image
	int texWidth = 0, texHeight = 0, numComponents = 0;
	stbi_uc *pPixels = stbi_load( path.string().c_str(), &texWidth, &texHeight, &numComponents, STBI_rgb_alpha );

	if ( !pPixels )
		throw std::runtime_error( "[Vulkan]Failed to load texture image!" );

	LoadRGBA( pPixels, texWidth, texHeight );
	stbi_image_free( pPixels );
}

void TextureVK::Load( const std::array< string, 6 > &faces )
{
	MemoryWriter memWriter;

	// TODO: We should make sure all faces of the cube are the same size
	int texWidth = 0, texHeight = 0, numComponents = 0;
	for ( size_t i = 0; i < faces.size(); ++i )
	{
		stbi_uc *pPixels = stbi_load( faces[ i ].c_str(), &texWidth, &texHeight, &numComponents, STBI_rgb_alpha );

		if ( !pPixels )
			throw std::runtime_error( "[Vulkan]Failed to load texture image!" );

		memWriter.write( ( char *)pPixels, ( size_t )( texWidth * texHeight * STBI_rgb_alpha ) );
		stprintf( "texWidth: %d texHeight: %d numComponents: %d\n", texWidth, texHeight, numComponents );
		stbi_image_free( pPixels );
	}

	m_nMipLevels = static_cast< uint32_t >( std::floor( std::log2( std::max( texWidth, texHeight ) ) ) ) + 1;

	VkDeviceSize imageSize = memWriter.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VulkanApp().createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;
	vkMapMemory( vulkan().device, stagingBufferMemory, 0, imageSize, 0, &pData );
		//std::memcpy( pData, ppPixels.data(), static_cast< size_t >( imageSize ) );
		std::memcpy( pData, memWriter.data(), static_cast< size_t >( imageSize ) );
	vkUnmapMemory( vulkan().device, stagingBufferMemory );

	VulkanApp().createImage( static_cast< uint32_t >( texWidth ),
			static_cast< uint32_t >( texHeight ),
			m_nMipLevels,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vkTextureImage,
			m_vkTextureImageMemory,
			true );

	VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_nMipLevels, true );
	VulkanApp().copyBufferToImage( stagingBuffer, m_vkTextureImage, static_cast< uint32_t >( texWidth ), static_cast< uint32_t >( texHeight ), static_cast< uint32_t >( STBI_rgb_alpha ), true );
	VulkanApp().generateMipMaps( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, m_nMipLevels, true );

	// TODO: If we don't generate mip maps, transition
	//VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_nMipLevels );

	vkDestroyBuffer( vulkan().device, stagingBuffer, nullptr );
	vkFreeMemory( vulkan().device, stagingBufferMemory, nullptr );

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_nMipLevels, VK_IMAGE_VIEW_TYPE_CUBE );

	// Create Texture Sampler
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( m_nMipLevels );

	if ( vkCreateSampler( vulkan().device, &samplerInfo, nullptr, &m_vkTextureSampler ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );
}

void TextureVK::LoadRGBA( const unsigned char *pPixels, size_t width, size_t height, bool bGenMipMaps /*= false*/ )
{
	constexpr const auto numChannels = 4;

	if ( bGenMipMaps )
		m_nMipLevels = static_cast< uint32_t >( std::floor( std::log2( std::max( width, height ) ) ) ) + 1;

	VkDeviceSize imageSize = width * height * numChannels;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VulkanApp().createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;
	vkMapMemory( vulkan().device, stagingBufferMemory, 0, imageSize, 0, &pData );
		std::memcpy( pData, pPixels, static_cast< size_t >( imageSize ) );
	vkUnmapMemory( vulkan().device, stagingBufferMemory );

	VulkanApp().createImage( static_cast< uint32_t >( width ),
			static_cast< uint32_t >( height ),
			m_nMipLevels,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vkTextureImage,
			m_vkTextureImageMemory );

	VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_nMipLevels );
	VulkanApp().copyBufferToImage( stagingBuffer, m_vkTextureImage, static_cast< uint32_t >( width ), static_cast< uint32_t >( height ), static_cast< uint32_t >( numChannels ) );

	// Generate Mipmaps
	if ( bGenMipMaps )
		VulkanApp().generateMipMaps( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, static_cast< int32_t >( width ), static_cast< int32_t >( height ), m_nMipLevels );
	else
		VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_nMipLevels );

	vkDestroyBuffer( vulkan().device, stagingBuffer, nullptr );
	vkFreeMemory( vulkan().device, stagingBufferMemory, nullptr );

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_nMipLevels, VK_IMAGE_VIEW_TYPE_2D );

	// Create Texture Sampler
	VkSamplerCreateInfo samplerInfo;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( m_nMipLevels );

	if ( vkCreateSampler( vulkan().device, &samplerInfo, nullptr, &m_vkTextureSampler ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );
}

void TextureVK::LoadKtx( const std::filesystem::path &ktxFile )
{
	ktxVulkanDeviceInfo kvdi;
	ktxTexture *pTexture = nullptr;

	ktxVulkanDeviceInfo_Construct( &kvdi, vulkan().physicalDevice, vulkan().device, vulkan().graphicsQueue, vulkan().commandPool, nullptr );

	auto CheckThrowError = [ & ]( KTX_error_code ktxResult )
	{
		if ( ktxResult != KTX_SUCCESS )
		{
			std::stringstream ss;
			ss << "[Vulkan]Failed to load KTX texture \"" << ktxFile.string() << "\": " << ktxErrorString( ktxResult );
			throw std::runtime_error( ss.str() );
		}
	};

	CheckThrowError( ktxTexture_CreateFromNamedFile( ktxFile.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pTexture ) );
	CheckThrowError( ktxTexture_VkUploadEx( pTexture,
		&kvdi,
		&m_ktxVulkanTexture,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ) );

	m_nMipLevels = m_ktxVulkanTexture.levelCount;

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_ktxVulkanTexture.image, m_ktxVulkanTexture.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_ktxVulkanTexture.levelCount, m_ktxVulkanTexture.viewType );

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( m_ktxVulkanTexture.levelCount );

	if ( vkCreateSampler( vulkan().device, &samplerInfo, nullptr, &m_vkTextureSampler ) != VK_SUCCESS )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );

	ktxTexture_Destroy( pTexture );
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

	if ( m_ktxVulkanTexture.image != VK_NULL_HANDLE && m_ktxVulkanTexture.deviceMemory != VK_NULL_HANDLE )
	{
		ktxVulkanTexture_Destruct( &m_ktxVulkanTexture, vulkan().device, nullptr );
		m_ktxVulkanTexture.image = VK_NULL_HANDLE;
		m_ktxVulkanTexture.deviceMemory = VK_NULL_HANDLE;
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