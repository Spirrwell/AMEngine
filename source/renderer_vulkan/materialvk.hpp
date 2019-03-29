#ifndef MATERIALVK_HPP
#define MATERIALVK_HPP

#include <vector>
#include <map>
#include <fstream>

#include "vulkan/vulkan.hpp"
#include "string.hpp"
#include "shadersystem/ishader.hpp"
#include "vulkan_interface.hpp"

class ShaderVK;
class TextureVK;

class MaterialVK : public vkApp::CVulkanInterface
{
    friend class ShaderVK;

public:
    MaterialVK( const string &materialPath );
	MaterialVK( std::ifstream &material );
    virtual ~MaterialVK();

	void Shutdown();

	void LoadMaterial( std::ifstream &material );

    TextureVK *GetTexture( const string &matParamName );
    ShaderVK *GetShader() const { return m_pShader; }

    std::vector< vk::DescriptorSet > m_vkDescriptorSets;
	vk::DescriptorPool m_vkDescriptorPool;

private:
    ShaderVK *m_pShader = nullptr;

    std::vector< MaterialParameter_t > m_MaterialParams;
    std::map< string, TextureVK* > m_mapTextures;
};

#endif // MATERIALVK_HPP