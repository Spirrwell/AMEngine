#ifndef SKYSHADER_HPP
#define SKYSHADER_HPP

#include "shadergl.hpp"
#include "mathdefs.hpp"

class SkyShader : public ShaderGL
{
public:
	using ShaderGL::ShaderGL;

	virtual void Initialize();
	virtual void Shutdown();
	virtual void Update( IMaterial *pMaterial );
};

#endif // SKYSHADER_HPP