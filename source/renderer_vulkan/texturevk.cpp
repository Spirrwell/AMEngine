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

void TextureVK::Load( const string &path )
{
	// Create Texture Image
	int texWidth = 0, texHeight = 0, numComponents = 0;
	stbi_uc *pPixels = stbi_load( path.c_str(), &texWidth, &texHeight, &numComponents, STBI_rgb_alpha );

	if ( !pPixels )
		throw std::runtime_error( "[Vulkan]Failed to load texture image!" );

	m_nMipLevels = static_cast< uint32_t >( std::floor( std::log2( std::max( texWidth, texHeight ) ) ) ) + 1;

	vk::DeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;
	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	VulkanApp().createBuffer( imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;
	vulkan().device.mapMemory( stagingBufferMemory, 0, imageSize, vk::MemoryMapFlags(), &pData );
		std::memcpy( pData, pPixels, static_cast< size_t >( imageSize ) );
	vulkan().device.unmapMemory( stagingBufferMemory );

	stbi_image_free( pPixels );
	VulkanApp().createImage( static_cast< uint32_t >( texWidth ),
			static_cast< uint32_t >( texHeight ),
			m_nMipLevels,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_vkTextureImage,
			m_vkTextureImageMemory );

	VulkanApp().transitionImageLayout( m_vkTextureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_nMipLevels );
	VulkanApp().copyBufferToImage( stagingBuffer, m_vkTextureImage, static_cast< uint32_t >( texWidth ), static_cast< uint32_t >( texHeight ), static_cast< uint32_t >( STBI_rgb_alpha ) );
	VulkanApp().generateMipMaps( m_vkTextureImage, vk::Format::eR8G8B8A8Unorm, texWidth, texHeight, m_nMipLevels );

	// TODO: If we don't generate mip maps, transition
	//VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_nMipLevels );

	// Generate Mipmaps

	vulkan().device.destroyBuffer( stagingBuffer, nullptr );
	vulkan().device.freeMemory( stagingBufferMemory, nullptr );

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_vkTextureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, m_nMipLevels, vk::ImageViewType::e2D );

	// Create Texture Sampler
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eNever;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( m_nMipLevels );

	auto[ result, sampler ] = vulkan().device.createSampler( samplerInfo, nullptr );

	if ( result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );

	m_vkTextureSampler = std::move( sampler );
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

	vk::DeviceSize imageSize = memWriter.size();
	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	VulkanApp().createBuffer( imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory );

	void *pData = nullptr;
	vulkan().device.mapMemory( stagingBufferMemory, 0, imageSize, vk::MemoryMapFlags(), &pData );
		//std::memcpy( pData, ppPixels.data(), static_cast< size_t >( imageSize ) );
		std::memcpy( pData, memWriter.data(), static_cast< size_t >( imageSize ) );
	vulkan().device.unmapMemory( stagingBufferMemory );

	VulkanApp().createImage( static_cast< uint32_t >( texWidth ),
			static_cast< uint32_t >( texHeight ),
			m_nMipLevels,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_vkTextureImage,
			m_vkTextureImageMemory,
			true );

	VulkanApp().transitionImageLayout( m_vkTextureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_nMipLevels, true );
	VulkanApp().copyBufferToImage( stagingBuffer, m_vkTextureImage, static_cast< uint32_t >( texWidth ), static_cast< uint32_t >( texHeight ), static_cast< uint32_t >( STBI_rgb_alpha ), true );
	VulkanApp().generateMipMaps( m_vkTextureImage, vk::Format::eR8G8B8A8Unorm, texWidth, texHeight, m_nMipLevels, true );

	// TODO: If we don't generate mip maps, transition
	//VulkanApp().transitionImageLayout( m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_nMipLevels );

	vulkan().device.destroyBuffer( stagingBuffer, nullptr );
	vulkan().device.freeMemory( stagingBufferMemory, nullptr );

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_vkTextureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, m_nMipLevels, vk::ImageViewType::eCube );

	// Create Texture Sampler
	vk::SamplerCreateInfo samplerInfo = {};
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( m_nMipLevels );

	auto[ result, sampler ] = vulkan().device.createSampler( samplerInfo, nullptr );

	if ( result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );

	m_vkTextureSampler = std::move( sampler );
}

void TextureVK::LoadKtx( const std::filesystem::path &ktxFile )
{
	if ( ktxFile.empty() || ktxFile.extension() != ".ktx" )
		throw std::runtime_error( "[Vulkan]Invalid KTX file path: " + ktxFile.string() );

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

	std::cout << "ktxFile: "  << ktxFile.string().c_str() << std::endl;
	CheckThrowError( ktxTexture_CreateFromNamedFile( ktxFile.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pTexture ) );
	std::cout << "CreateFromNamedFile worked\n";
	CheckThrowError( ktxTexture_VkUploadEx( pTexture,
		&kvdi,
		&m_ktxVulkanTexture,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ) );

	m_nMipLevels = m_ktxVulkanTexture.levelCount;

	// Create Texture Image View
	m_vkTextureImageView = VulkanApp().createImageView( m_ktxVulkanTexture.image, vk::Format( m_ktxVulkanTexture.imageFormat ), vk::ImageAspectFlagBits::eColor, m_ktxVulkanTexture.levelCount, vk::ImageViewType( m_ktxVulkanTexture.viewType ) );

	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;

	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( m_ktxVulkanTexture.levelCount );

	auto[ result, sampler ] = vulkan().device.createSampler( samplerInfo, nullptr );

	if ( result != vk::Result::eSuccess )
		throw std::runtime_error( "[Vulkan]Failed to create texture sampler!" );

	m_vkTextureSampler = std::move( sampler );

	ktxTexture_Destroy( pTexture );
}

void TextureVK::Shutdown()
{
	if ( m_vkTextureSampler )
	{
		vulkan().device.destroySampler( m_vkTextureSampler, nullptr );
		m_vkTextureSampler = nullptr;
	}

	if ( m_vkTextureImageView )
	{
		vulkan().device.destroyImageView( m_vkTextureImageView, nullptr );
		m_vkTextureImageView = nullptr;
	}

	if ( m_ktxVulkanTexture.image != VK_NULL_HANDLE && m_ktxVulkanTexture.deviceMemory != VK_NULL_HANDLE )
	{
		ktxVulkanTexture_Destruct( &m_ktxVulkanTexture, vulkan().device, nullptr );
		m_ktxVulkanTexture.image = VK_NULL_HANDLE;
		m_ktxVulkanTexture.deviceMemory = VK_NULL_HANDLE;
	}

	if ( m_vkTextureImage )
	{
		vulkan().device.destroyImage( m_vkTextureImage, nullptr );
		m_vkTextureImage = nullptr;
	}

	if ( m_vkTextureImageMemory )
	{
		vulkan().device.freeMemory( m_vkTextureImageMemory, nullptr );
		m_vkTextureImageMemory = nullptr;
	}
}