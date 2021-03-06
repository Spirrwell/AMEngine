#ifndef ISHADERMANAGER_H
#define ISHADERMANAGER_H

#include "ishader.hpp"
#include "ibaseshader.hpp"

class IShaderManager
{
public:
	virtual bool Init() = 0;
	virtual void InserShader( IBaseShader *pShader ) = 0;

	virtual IShader *CreateShaderObject( const std::string &strShaderName ) = 0;
	virtual void DeleteShaderObject( IShader *pShader ) = 0;
};

#define SHADERMANAGER_INTERFACE "SHADERMANAGER_V001"

#endif // ISHADERMANAGER_H