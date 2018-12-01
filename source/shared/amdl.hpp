#ifndef AMDL_HPP
#define AMDL_HPP

#include <vector>
#include <optional>
#include <fstream>
#include <stdint.h>

#include "vertex.hpp"
#include "string.hpp"

namespace AMDL
{
	struct MeshData;

	typedef std::vector< Vertex > Vertices;
	typedef std::vector< uint32_t > Indices;
	typedef std::vector< MeshData > Meshes;
	typedef std::vector< string > MaterialPaths;

	struct MeshData
	{
		Vertices vertices;
		Indices indices;
		std::optional< string > materialName;
	};

	struct ModelData
	{
		Meshes meshes;
		MaterialPaths materialPaths;
	};

	inline void ReadAMDLFile( const string &modelPath, ModelData &outData )
	{
		printf( "Reading AMDL file: %s\n", modelPath.c_str() );

		std::ifstream amdlFile;
		amdlFile.open( modelPath, std::ifstream::binary );

		if ( !amdlFile.good() )
			throw std::runtime_error( "Failed to load AMDL" );

		uint32_t version = 0;
		amdlFile.read( ( char* )&version, sizeof( version ) );

		uint32_t numMaterialPaths = 0;
		amdlFile.read( ( char* )&numMaterialPaths, sizeof( numMaterialPaths ) );

		MaterialPaths materialPaths;
		materialPaths.resize( static_cast< size_t >( numMaterialPaths ) );

		for ( uint32_t i = 0; i < numMaterialPaths; ++i )
		{
			uint32_t pathLen = 0;
			amdlFile.read( ( char* )&pathLen, sizeof( pathLen ) );

			materialPaths[ i ].resize( pathLen, '\0' );
			amdlFile.read( materialPaths[ i ].data(), static_cast< std::streamsize >( pathLen ) );
		}

		uint32_t numMeshes = 0;
		amdlFile.read( ( char* )&numMeshes, sizeof( numMeshes ) );

		Meshes meshes;
		meshes.resize( static_cast< size_t >( numMeshes ) );

		for ( uint32_t i = 0; i < numMeshes; ++i )
		{
			uint32_t numVertices = 0;
			amdlFile.read( ( char* )&numVertices, sizeof ( numVertices ) );

			if ( numVertices > 0 )
			{
				std::vector< float > positions;	// 1 x 3
				std::vector< float > texCoords;	// 1 x 2
				std::vector< float > normals;	// 1 x 3

				bool bHasPositions = false;
				amdlFile.read( ( char* )&bHasPositions, sizeof( bHasPositions ) );

				if ( bHasPositions )
				{
					positions.resize( static_cast< size_t >( numVertices ) * 3 );
					amdlFile.read( ( char* )positions.data(), sizeof( positions[ 0 ] ) * positions.size() );
				}

				bool bHasTexCoords = false;
				amdlFile.read( ( char* )&bHasTexCoords, sizeof( bHasTexCoords ) );

				if ( bHasTexCoords )
				{
					texCoords.resize( static_cast< size_t >( numVertices ) * 2 );
					amdlFile.read( ( char* )texCoords.data(), sizeof( texCoords[ 0 ] ) * texCoords.size() );
				}

				bool bHasNormals = false;
				amdlFile.read( ( char* )&bHasNormals, sizeof( bHasNormals ) );

				if ( bHasNormals )
				{
					normals.resize( static_cast< size_t >( numVertices ) * 3 );
					amdlFile.read( ( char* )normals.data(), sizeof( normals[ 0 ] ) * normals.size() );
				}

				Vertices vertices;
				vertices.resize( static_cast< size_t >( numVertices ) );

				for ( uint32_t v = 0; v < numVertices; ++v )
				{
					if ( bHasPositions )
						vertices[ v ].pos = { positions[ v * 3 ], positions[ v * 3 + 1 ], positions[ v * 3 + 2 ] };
					if ( bHasTexCoords )
						vertices[ v ].texCoord = { texCoords[ v * 2 ], texCoords[ v * 2 + 1 ] };
					if ( bHasNormals )
						vertices[ v ].normal = { normals[ v * 3 ], normals[ v * 3 + 1 ], normals[ v * 3 + 2 ] };
				}

				uint32_t numIndices = 0;
				amdlFile.read( ( char* )&numIndices, sizeof( numIndices ) );

				Indices indices;
				indices.resize( static_cast< size_t >( numIndices ) );

				amdlFile.read( ( char* )indices.data(), sizeof( indices[ 0 ] ) * numIndices );

				bool bHasMaterial = false;
				amdlFile.read( ( char* )&bHasMaterial, sizeof( bHasMaterial ) );
				std::optional< string > materialName = std::nullopt;

				if ( bHasMaterial )
				{
					uint32_t matNameLen = 0;
					amdlFile.read( ( char* )&matNameLen, sizeof( matNameLen ) );

					materialName = string( static_cast< size_t >( matNameLen ), '\0' );
					amdlFile.read( materialName->data(), static_cast< std::streamsize >( matNameLen ) );
				}

				meshes[ i ] = { vertices, indices, materialName };
			}
		}
		
		outData.meshes = meshes;
		outData.materialPaths = materialPaths;
	}
}

#endif // AMDL_HPP