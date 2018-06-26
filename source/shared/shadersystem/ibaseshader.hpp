#ifndef IBASESHADER_HPP
#define IBASESHADER_HPP

#include "materialsystem/imaterial.hpp"
#include "ishader.hpp"
#include "string.hpp"

#include <vector>

class IBaseShader
{
public:
	virtual bool Init() = 0;
	virtual void InitShaderParams() = 0;
	virtual void Draw( IMaterial *pMaterial ) = 0;

	virtual void BindShader() = 0;
	virtual void Shutdown() = 0;

	virtual IShader *GetIShader() = 0;

	virtual std::vector < MaterialParameter_t > GetMaterialParameters() = 0;

	virtual const string &GetName() = 0;
};

#endif // IBASESHADER_HPP