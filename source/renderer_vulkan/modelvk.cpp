#include "modelvk.hpp"
#include "amdl.hpp"
#include "platform.hpp"
#include "materialvk.hpp"
#include "meshvk.hpp"

#include <fstream>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

ModelVK::~ModelVK()
{
	Shutdown();
}

void ModelVK::Shutdown()
{
	for ( auto &pMesh : m_pMeshes )
	{
		delete pMesh->GetMaterial();
		delete pMesh;
	}

	m_pMeshes.clear();
}

void ModelVK::LoadModel( const string &modelPath )
{
	AMDL::ModelData modelData;
	AMDL::ReadAMDLFile( modelPath, modelData );

	m_pMeshes.resize( modelData.meshes.size(), nullptr );

	for ( size_t i = 0; i < modelData.meshes.size(); ++i )
	{
		const auto &meshData = modelData.meshes[ i ];

		if ( meshData.materialName )
		{
			MaterialVK *pMaterial = nullptr;
			for ( const auto &matPath : modelData.materialPaths )
			{
				std::ifstream testPath;
				testPath.open( string( GAME_DIR ) + matPath + "/" + meshData.materialName.value() + ".amat" );

				if ( testPath.is_open() )
				{
					pMaterial = new MaterialVK( testPath );
					break;
				}
			}

			// TODO: We should reduce the number of copies we make of the indices and vertices
			MeshVK *pMesh = new MeshVK( meshData.vertices, meshData.indices, pMaterial );
			m_pMeshes[ i ] = pMesh;
		}
	}
}