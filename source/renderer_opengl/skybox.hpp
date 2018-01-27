#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <GL/glew.h>

#include "shadersystem/ishader.hpp"
#include "shadersystem/baseshader.hpp"
#include "materialsystem/imaterial.hpp"

#include "amlib/transform.hpp"

class Skybox
{
public:
	Skybox();
	virtual ~Skybox();

	bool IsValid() { return ( m_pShader != nullptr && m_pMaterial != nullptr ); }

	void Draw();

private:
	IBaseShader *m_pShader;
	IMaterial *m_pMaterial;

	GLuint m_VertexArrayObject;
	GLuint m_VertexArrayBuffer;
};

#endif // SKYBOX_HPP