#ifndef GENERICSHADER_HPP
#define GENERICSHADER_HPP

#include "shadergl.hpp"
#include "mathdefs.hpp"

class GenericShader : public ShaderGL
{
public:
	using ShaderGL::ShaderGL;

	virtual void Initialize() override;
	virtual void Update( IMaterial *pMaterial ) override;

};

#endif // GENERICSHADER_HPP