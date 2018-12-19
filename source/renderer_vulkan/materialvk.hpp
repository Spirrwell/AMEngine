#ifndef MATERIALVK_HPP
#define MATERIALVK_HPP

#include <vector>
#include <map>

#include "vulkan/vulkan.hpp"
#include "string.hpp"
#include "shadersystem/ishader.hpp"

class ShaderVK;
class TextureVK;

class MaterialVK
{
    friend class ShaderVK;

public:
    MaterialVK( const string &materialPath );
    virtual ~MaterialVK();

    TextureVK *GetTexture( const string &matParamName );
    ShaderVK *GetShader() const { return m_pShader; }

    std::vector< VkDescriptorSet > m_vkDescriptorSets;

private:
    ShaderVK *m_pShader = nullptr;

    std::vector< MaterialParameter_t > m_MaterialParams;
    std::map< string, TextureVK* > m_mapTextures;
};

#endif // MATERIALVK_HPP