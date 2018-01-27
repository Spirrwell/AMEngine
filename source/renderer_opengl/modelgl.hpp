#ifndef MODELGL_HPP
#define MODELGL_HPP

#include <string>
#include <vector>

#include "imodel.hpp"
#include "meshgl.hpp"

class ModelGL : public IModel
{
public:
	ModelGL( const std::string &modelFile );
	virtual ~ModelGL();

	void Draw() override;
	unsigned int GetModelIndex() override { return m_iModelIndex; }

	Transform transform;

private:
	std::vector < MeshGL* > m_pMeshes;
	unsigned int m_iModelIndex;
};

#endif // MODELGL_HPP