#include <iostream>
#include <vector>
#include <fstream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "string.hpp"
#include "platform.hpp"
#include "renderer/vertex.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

struct AMDLMesh;

typedef std::vector< Vertex > Vertices;
typedef std::vector< uint32_t > Indices;
typedef std::vector< string > Materials;
typedef std::vector< string > MaterialPaths;
typedef std::vector< AMDLMesh > Meshes;

typedef string Material;
typedef string MaterialPath;
typedef uint32_t Index;

struct AMDLMesh
{
	Vertices vertices;
	Indices indices;
	Material material;
};

struct AMDLModel
{
	Meshes meshes;
	MaterialPaths materialPaths;
};

void WriteAMDLFile( const AMDLModel &model, const string &output )
{
	uint32_t version = 0;
	uint32_t numMaterialPaths = static_cast< uint32_t >( model.materialPaths.size() );

	std::ofstream amdlFile;
	amdlFile.open( output, std::ofstream::binary );

	amdlFile.write( ( char* )&version, sizeof( version ) );
	amdlFile.write( ( char* )&numMaterialPaths, sizeof( numMaterialPaths ) );

	for ( uint32_t i = 0; i < numMaterialPaths; ++i )
	{
		uint32_t pathLen = static_cast< uint32_t >( model.materialPaths[ 0 ].size() );
		amdlFile.write( ( char* )&pathLen, sizeof( pathLen ) );
		amdlFile.write( model.materialPaths[ i ].data(), static_cast< std::streamsize >( pathLen ) );
	}

	uint32_t numMeshes = static_cast< uint32_t >( model.meshes.size() );
	amdlFile.write( ( char* )&numMeshes, sizeof( numMeshes ) );

	for ( uint32_t i = 0; i < numMeshes; ++i )
	{
		std::vector< float > positions;	// 1 x 3
		std::vector< float > texCoords;	// 1 x 2
		std::vector< float > normals;	// 1 x 3

		uint32_t numVertices = static_cast< uint32_t >( model.meshes[ i ].vertices.size() );
		amdlFile.write( ( char* )&numVertices, sizeof( numVertices ) );

		for ( const Vertex &vertex : model.meshes[ i ].vertices )
		{
			positions.push_back( vertex.pos.x );
			positions.push_back( vertex.pos.y );
			positions.push_back( vertex.pos.z );

			texCoords.push_back( vertex.texCoord.x );
			texCoords.push_back( vertex.texCoord.y );

			normals.push_back( vertex.normal.x );
			normals.push_back( vertex.normal.y );
			normals.push_back( vertex.normal.z );
		}

		// These should all be true, but for the future we'll write these booleans in the AMDL file
		bool bHasPositions = positions.size() > 0;
		bool bHasTexCoords = texCoords.size() > 0;
		bool bHasNormals = normals.size() > 0;

		amdlFile.write( ( char* )&bHasPositions, sizeof( bHasPositions ) );

		if ( bHasPositions )
			amdlFile.write( ( char* )positions.data(), sizeof( positions[ 0 ] ) * positions.size() );

		amdlFile.write( ( char* )&bHasTexCoords, sizeof( bHasTexCoords ) );

		if ( bHasTexCoords )
			amdlFile.write( ( char* )texCoords.data(), sizeof( texCoords[ 0 ] ) * texCoords.size() );

		amdlFile.write( ( char* )&bHasNormals, sizeof( bHasNormals ) );

		if ( bHasNormals )
			amdlFile.write( ( char* )normals.data(), sizeof( normals[ 0 ] ) * normals.size() );

		uint32_t numIndices = static_cast< uint32_t >( model.meshes[ i ].indices.size() );

		amdlFile.write( ( char* )&numIndices, sizeof( numIndices ) );
		amdlFile.write( ( char* )model.meshes[ i ].indices.data(), sizeof( Index ) * static_cast< std::streamsize >( numIndices ) );

		bool bHasMaterial = !model.meshes[ i ].material.empty();
		amdlFile.write( ( char* )&bHasMaterial, sizeof( bHasMaterial ) );

		if ( bHasMaterial )
		{
			uint32_t matNameLen = static_cast< uint32_t >( model.meshes[ i ].material.size() );

			amdlFile.write( ( char* )&matNameLen, sizeof( matNameLen ) );
			amdlFile.write( model.meshes[ i ].material.data(), static_cast< std::streamsize >( matNameLen ) );
		}
	}

	amdlFile.close();
}

int main( int argc, char *argv[] )
{
	string inputModelFile;

	if ( argc > 1 )
		inputModelFile = argv[ 1 ];
	else
	{
		std::cout << "Please enter the path to the model you would like to build into an amdl:\n";
		std::getline( std::cin, inputModelFile );
	}

	Assimp::Importer Importer;
	const aiScene *pScene = Importer.ReadFile( inputModelFile, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs );

	Meshes meshes;

	for ( unsigned int i = 0; i < pScene->mNumMeshes; ++i )
	{
		aiMesh *pAIMesh = pScene->mMeshes[ i ];

		Indices indices;
		Vertices vertices;
		Material material;

		for ( unsigned int j = 0; j < pAIMesh->mNumVertices; ++j )
		{
			Vertex vertex;

			if ( pAIMesh->HasPositions() )
				vertex.pos = { pAIMesh->mVertices[ j ].x, pAIMesh->mVertices[ j ].y, pAIMesh->mVertices[ j ].z };
			if ( pAIMesh->HasTextureCoords( 0 ) )
				vertex.texCoord = { pAIMesh->mTextureCoords[ 0 ][ j ].x, pAIMesh->mTextureCoords[ 0 ][ j ].y };
			if ( pAIMesh->HasNormals() )
				vertex.normal = { pAIMesh->mNormals[ j ].x, pAIMesh->mNormals[ j ].y, pAIMesh->mNormals[ j ].z };

			vertices.push_back( vertex );
		}

		for ( unsigned int j = 0; j < pAIMesh->mNumFaces; ++j )
		{
			for ( unsigned int k = 0; k < pAIMesh->mFaces[ j ].mNumIndices; ++k )
				indices.push_back( pAIMesh->mFaces[ j ].mIndices[ k ] );
		}

		if ( pAIMesh->mMaterialIndex >= 0 )
		{
			aiMaterial *pAIMaterial = pScene->mMaterials[ pAIMesh->mMaterialIndex ];

			if ( pAIMaterial != nullptr )
			{
				aiString matName;

				pAIMaterial->Get( AI_MATKEY_NAME, matName );
				material = matName.C_Str();
			}
		}

		meshes.push_back( { vertices, indices, material } );
	}

	MaterialPaths materialPaths;
	std::cout << "Please enter material paths or type \"done\" when finished:\n";

	while( true )
	{
		string response;
		std::getline( std::cin, response );

		if ( response == "done" )
			break;

		materialPaths.push_back( response );
	}

	AMDLModel amdlModel = { meshes, materialPaths };
	
	size_t totalIndices = 0;
	size_t totalVerts = 0;
	size_t numMaterials = 0;

	for ( const auto &mesh : amdlModel.meshes )
	{
		totalVerts += mesh.vertices.size();
		totalIndices += mesh.indices.size();
		if ( !mesh.material.empty() )
			numMaterials++;
	}

	std::cout << "Vertex Count: " << totalVerts << std::endl;
	std::cout << "Index Count: " << totalIndices << std::endl;
	std::cout << "Material Count: " << numMaterials << std::endl;

	string outputFile;

	if ( argc > 2 )
		outputFile = argv[ 2 ];
	else
	{
		std::cout << "Please enter an output path for your model file:\n";
		std::getline( std::cin, outputFile );
	}

	WriteAMDLFile( amdlModel, outputFile );

	std::cin.get();
	return 0;
}