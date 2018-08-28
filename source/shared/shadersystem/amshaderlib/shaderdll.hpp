#ifndef SHADERDLL_HPP
#define SHADERDLL_HPP

#include <vector>
#include <exception>

#include "ibaseshader.hpp"
#include "shadersystem/ishadermanager.hpp"
#include "interface.hpp"
#include "renderer/irenderer.hpp"

class CShaderDLL : public DLLInterfaceOverride
{
public:
	CShaderDLL();
	virtual ~CShaderDLL();

	bool Init() override;
	void Shutdown() override;

	void InsertShader( IBaseShader *pShader );
	IBaseShader *GetShader( size_t iShader );

	//std::vector < IBaseShader * > GetShaders() { return m_pShaders; }

private:
	std::vector < IBaseShader * > m_pShaders;
};

extern IRenderer *g_pRenderer;
extern IShaderManager *g_pShaderManager;

extern CShaderDLL *GetShaderDLLInternal();

#endif // SHADERDLL_HPP