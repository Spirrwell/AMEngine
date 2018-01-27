#ifndef SHADERGL_HPP
#define SHADERGL_HPP

#include <map>

#include "shadersystem/ishader.hpp"
#include "renderer/irenderer.hpp"
#include "renderer/itexture.hpp"

#include "GL/glew.h"
#include "SDL_opengl.h"

#include "amlib/transform.hpp"

extern IRenderer *g_pRenderer;

#define VERTEXSHADER_EXTENSION ".vert"
#define FRAGMENTSHADER_EXTENSION ".frag"

#define SPIRV_EXTENSION ".spv"

enum ShaderType
{
	VERTEX_SHADER = 0,
	FRAGMENT_SHADER = 1,
	NUM_SHADERS
};

class ShaderGL : public IShader
{
public:
	ShaderGL( const std::string &strShaderName );
	virtual ~ShaderGL();

	virtual void Initialize();
	void LinkAndValidate();

	virtual void Update( IMaterial *pMaterial ) {}

	const std::string &GetName();
	unsigned int GetIndex();

	virtual std::vector < MaterialParameter_t > GetMaterialParameters();

	// Used for setting non-opaque uniform types, meaning they are part of a uniform block
	// We're using SPIR-V requires these uniforms to to be set as part of a uniform block
	void SetMatrix3f( const std::string &uniform, Matrix3f mat3x3 ) override;
	void SetMatrix4f( const std::string &uniform, Matrix4f mat4x4 ) override;
	void SetVector2f( const std::string &uniform, Vector2f vec2 ) override;
	void SetVector3f( const std::string &uniform, Vector3f vec3 ) override;
	void SetVector4f( const std::string &uniform, Vector4f vec4 ) override;
	void SetFloat( const std::string &uniform, float flValue ) override;
	void SetInt( const std::string &uniform, int iValue ) override;

	// Used for setting opaque uniform types, which cannot be part of a uniform block
	void SetSampler( const std::string &uniform, int iValue ) override;

	void BindUBO( const std::string &strBlockName ) override;
	void UnbindUBO() override;

	void BindShader();
	virtual void Shutdown();

	void CreateUniformBlock( const std::string &strBlockName ) override;

	void AddUniformToBlock( const std::string &strBlockName, const std::string &strUniformName ) override;
	void AddUniform( const std::string &strUniformName ) override;

protected:

	std::string m_strShaderName;
	unsigned int m_iShaderIndex;

	GLuint m_hProgram;
	GLuint m_iShaders[NUM_SHADERS];

	std::string m_strCurrentUBO;

	std::map < std::string, GLint > m_mapUniforms;

	std::map < std::string, GLint > m_mapUniformOffsets;
	std::map < std::string, GLuint > m_mapUBOHandles;

	std::vector < MaterialParameter_t > m_vMaterialParameters;

	//IRenderer *m_pRenderer;
};

#endif // SHADERGL_HPP