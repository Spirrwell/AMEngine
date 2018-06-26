#include "modelgl.hpp"

//#include "assimp/Importer.hpp"
//#include "assimp/scene.h"
//#include "assimp/postprocess.h"

#include "platform.hpp"
#include "materialsystem/imaterialsystem.hpp"

#include <fstream>

extern IMaterialSystem *g_pMaterialSystem;

ModelGL::ModelGL( const string &modelFile )
{
	/*Assimp::Importer Importer;
	const aiScene *pScene = Importer.ReadFile( modelFile, aiProcess_Triangulate | aiProcess_FlipUVs  );

	for ( unsigned int i = 0; i < pScene->mNumMeshes; i++ )
	{
		aiMesh *pAIMesh = pScene->mMeshes[i];

		std::vector < unsigned int > Indices;
		std::vector < Vertex > Vertices;
		MaterialGL *pMaterial = nullptr;

		for ( unsigned int i = 0; i < pAIMesh->mNumVertices; i++ )
			Vertices.push_back(
				Vertex(
					Vector3f(
						pAIMesh->mVertices[i].x,
						pAIMesh->mVertices[i].y,
						pAIMesh->mVertices[i].z ),
					Vector2f(
						pAIMesh->mTextureCoords[0][i].x,
						pAIMesh->mTextureCoords[0][i].y ) ) );

		for( unsigned int i = 0; i < pAIMesh->mNumFaces; i++ )
		{
			for( unsigned int j = 0; j < pAIMesh->mFaces[i].mNumIndices; j++ )
				Indices.push_back( pAIMesh->mFaces[i].mIndices[j] );
		}

		if ( pAIMesh->mMaterialIndex >= 0 )
		{
			aiMaterial *pAIMaterial = pScene->mMaterials[ pAIMesh->mMaterialIndex ];

			if ( pAIMaterial != nullptr )
			{
				aiString matName;

				pAIMaterial->Get( AI_MATKEY_NAME, matName );
				pMaterial = new MaterialGL( string( GAME_DIR ) + "models/" + string( matName.C_Str() ) + ".amat" );
			}
		}

		m_pMeshes.push_back( new MeshGL( Vertices, Indices, pMaterial ) );
	}*/

	uint32_t version = 0;
	uint32_t numMaterialPaths = 0;

	std::ifstream amdlFile;
	amdlFile.open( modelFile, std::ios::binary );

	amdlFile.read( ( char * )&version, sizeof( version ) );
	amdlFile.read( ( char * )&numMaterialPaths, sizeof( numMaterialPaths ) );

	std::vector < string > matPaths;

	for ( uint32_t i = 0; i < numMaterialPaths; i++ )
	{
		uint32_t pathLen = 0;

		amdlFile.read( ( char * )&pathLen, sizeof ( pathLen ) );

		string path( pathLen, ' ' );
		amdlFile.read( path.data(), pathLen );

		matPaths.push_back( path );
	}

	uint32_t numMeshes = 0;
	amdlFile.read( ( char * )&numMeshes, sizeof( numMeshes ) );

	for ( uint32_t i = 0; i < numMeshes; i++ )
	{
		std::vector < uint32_t > indices;
		std::vector < float > positions;	// 1 x 3
		std::vector < float > texCoords;	// 1 x 2
		std::vector < float > normals;		// 1 x 3

		IMaterial *pMaterial = nullptr;

		uint32_t numVertices = 0;
		bool bHasPositions = false;

		amdlFile.read( ( char * )&numVertices, sizeof( numVertices ) );
		amdlFile.read( ( char * )&bHasPositions, sizeof( bHasPositions ) );

		if ( bHasPositions )
		{
			for ( uint32_t v = 0; v < numVertices; v++ )
			{
				float x, y, z;

				amdlFile.read( ( char * )&x, sizeof ( x ) );
				amdlFile.read( ( char * )&y, sizeof ( y ) );
				amdlFile.read( ( char * )&z, sizeof ( z ) );

				positions.push_back( x );
				positions.push_back( y );
				positions.push_back( z );
			}
		}

		bool bHasTexCoords = false;
		amdlFile.read( ( char * )&bHasTexCoords, sizeof( bHasTexCoords ) );

		if ( bHasTexCoords )
		{
			for ( uint32_t v = 0; v < numVertices; v++ )
			{
				float x, y;

				amdlFile.read( ( char * )&x, sizeof ( x ) );
				amdlFile.read( ( char * )&y, sizeof ( y ) );

				texCoords.push_back( x );
				texCoords.push_back( y );
			}
		}

		bool bHasNormals = false;
		amdlFile.read( ( char * )&bHasNormals, sizeof( bHasNormals ) );

		if ( bHasNormals )
		{
			for ( uint32_t v = 0; v < numVertices; v++ )
			{
				float x, y, z;

				amdlFile.read( ( char * )&x, sizeof ( x ) );
				amdlFile.read( ( char * )&y, sizeof ( y ) );
				amdlFile.read( ( char * )&z, sizeof ( z ) );

				normals.push_back( x );
				normals.push_back( y );
				normals.push_back( z );
			}
		}

		uint32_t numIndices = 0;
		amdlFile.read( ( char * )&numIndices, sizeof ( numIndices ) );

		for ( uint32_t j = 0; j < numIndices; j++ )
		{
			uint32_t index = 0;
			amdlFile.read( ( char * )&index, sizeof( index ) );

			indices.push_back( index );
		}

		bool bHasMaterial = false;
		amdlFile.read( ( char * )&bHasMaterial, sizeof( bHasMaterial ) );

		if ( bHasMaterial )
		{
			uint32_t matNameLen = 0;
			amdlFile.read( ( char * )&matNameLen, sizeof( matNameLen ) );

			string matName( matNameLen, ' ' );
			//matName.resize( matNameLen );
			amdlFile.read( matName.data(), matNameLen );

			for ( unsigned int x = 0; x < matPaths.size(); x++ )
			{
				string path = string( GAME_DIR ) + matPaths[x] + "/" + matName + ".amat";
				std::ifstream f( path );

				if ( f.good() )
				{
					f.close();
					pMaterial = g_pMaterialSystem->CreateMaterial( matPaths[x] + "/" + matName + ".amat" );
					break;
				}
			}
		}

		std::vector < Vertex > vertices;

		for ( uint32_t v = 0; v < numVertices; v++ )
		{
			if ( bHasPositions && bHasTexCoords && bHasNormals )
			{
				vertices.push_back(
					Vertex(
						Vector3f( positions[v * 3], positions[v * 3 + 1], positions[v * 3 + 2] ),
						Vector2f( texCoords[v * 2], texCoords[v * 2 + 1] ),
						Vector3f( normals[v * 3], normals[v * 3 + 1], normals[v * 3 + 2] ) ) );
			}
			else if ( bHasPositions && bHasTexCoords )
			{
				vertices.push_back(
					Vertex(
						Vector3f( positions[v * 3], positions[v * 3 + 1], positions[v * 3 + 2] ),
						Vector2f( texCoords[v * 2], texCoords[v * 2 + 1] ) ) );
			}
			else if ( bHasPositions )
			{
				vertices.push_back(
					Vertex(
						Vector3f( positions[v * 3], positions[v * 3 + 1], positions[v * 3 + 2] ) ) );
			}
		}

		if ( pMaterial == nullptr )
			pMaterial = g_pMaterialSystem->CreateMaterial( "materials/cube/BasicMaterial.amat" );

		m_pMeshes.push_back( new MeshGL( vertices, indices, pMaterial ) );
	}

	uint32_t totalVerts = 0;
	for ( MeshGL *pMesh : m_pMeshes )
		totalVerts += pMesh->GetVertexCount();

	printf( "Total number of vertices: %d\n", totalVerts );
}

ModelGL::~ModelGL()
{
	for ( MeshGL *pMesh : m_pMeshes ) delete pMesh;
}

void ModelGL::Draw()
{
	for ( MeshGL *pMesh : m_pMeshes )
	{
		pMesh->SetModelMatrix( transform.GetModelMatrix() );
		pMesh->Draw();
	}
}
