#ifndef SHADERMANAGERGL_HPP
#define SHADERMANAGERGL_HPP

#include "ishadermanager.hpp"

class ShaderManagerGL : public IShaderManager
{
public:
	bool Init() override;
	void InserShader( IBaseShader *pShader ) override;

	IShader *CreateShaderObject( const string &strShaderName ) override;

private:
	std::vector < IBaseShader * > m_pShaders;
};

#endif // SHADERMANAGERGL_HPP