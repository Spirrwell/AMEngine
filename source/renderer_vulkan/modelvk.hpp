#ifndef MODELVK_HPP
#define MODELVK_HPP

#include <vector>
#include <filesystem>

#include "string.hpp"
#include "mathdefs.hpp"

class MeshVK;

class ModelVK
{
public:

	virtual ~ModelVK();
	void Shutdown();

	void LoadModel( const std::filesystem::path &modelPath );

	const std::vector< MeshVK* > &GetMeshes() const { return m_pMeshes; }
	const Matrix4f &GetModelMatrix() const { return m_mat4Model; }
	void SetModelMatrix( Matrix4f model ) { m_mat4Model = model; }

private:
	std::vector< MeshVK* > m_pMeshes;
	Matrix4f m_mat4Model = {};
};

#endif // MODELVK_HPP