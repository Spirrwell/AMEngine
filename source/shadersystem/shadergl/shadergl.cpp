#include <fstream>

#include "shadergl.hpp"
#include "platform.hpp"
#include "factory/ifactory.hpp"

std::vector < char > ReadBinaryShader( const std::string &strFileName, ShaderType eShaderType )
{
	std::string strShaderExtension;

	switch( eShaderType )
	{
	case VERTEX_SHADER:
		strShaderExtension = std::string( VERTEXSHADER_EXTENSION ) + std::string ( SPIRV_EXTENSION );
		break;
	case FRAGMENT_SHADER:
		strShaderExtension = std::string( FRAGMENTSHADER_EXTENSION ) + std::string( SPIRV_EXTENSION );
		break;
	default:
		// Something bad happened
		return std::vector< char >();
	}

	std::string fullPath = std::string( GAME_DIR ) + std::string( "shaders/" ) + strFileName + strShaderExtension;
	std::ifstream file( fullPath, std::ios::ate | std::ios::binary );

	size_t fileSize = ( size_t )file.tellg();
	std::vector< char > shader( fileSize );

	file.seekg( 0 );
	file.read( shader.data(), fileSize );

	file.close();

	return shader;
}

std::string ReadShader( const std::string &strFileName, ShaderType eShaderType )
{
	std::string strShaderExtension;

	switch( eShaderType )
	{
	case VERTEX_SHADER:
		strShaderExtension = VERTEXSHADER_EXTENSION;
		break;
	case FRAGMENT_SHADER:
		strShaderExtension = FRAGMENTSHADER_EXTENSION;
		break;
	default:
		// Something bad happened
		return "";
	}

	std::ifstream file;
	std::string fullPath = std::string( GAME_DIR ) + std::string( "shaders/" ) + strFileName + strShaderExtension;
	file.open( fullPath );

	std::string output, line;

	if ( file.is_open() )
	{
		while ( file.good() )
		{
			std::getline( file, line );
			output.append( line + "\n" );
		}
	}
	else
		printf( "Unable to load shader: %s\n", strFileName.c_str() );

	return output;
}

void CheckShaderError( GLuint shader, GLuint flag, bool isProgram, const std::string &errorMessage )
{
    GLint success = 0;
    GLint infoLogLen = 0;

    if ( isProgram )
		glGetProgramiv( shader, flag, &success );
	else
		glGetShaderiv( shader, flag, &success );

	if ( success == GL_FALSE )
	{
		if ( isProgram )
		{
			glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &infoLogLen );
			std::vector< GLchar > infoLog( infoLogLen );

			glGetProgramInfoLog( shader, infoLogLen, &infoLogLen, infoLog.data() );
			printf( "%s: %s\n", errorMessage.c_str(), infoLog.data() );
		}
		else
		{
			glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLogLen );
			std::vector< GLchar > infoLog( infoLogLen );

			glGetShaderInfoLog( shader, infoLogLen, &infoLogLen, infoLog.data() );
			printf( "%s: %s\n", errorMessage.c_str(), infoLog.data() );
		}

	}
}

GLuint CreateShader( const std::string &text, GLenum shaderType )
{
	GLuint shader = glCreateShader( shaderType );

	if ( shader == 0 )
	{
		printf( "Fatal Error: Shader creation failed!\n" );
		return 0;
	}

	const GLchar *pszShaderSource[ 1 ];
	GLint iShaderSourceStringLengths[ 1 ];

	pszShaderSource[ 0 ] = text.c_str();
	iShaderSourceStringLengths[ 0 ] = ( GLint )text.length();

	glShaderSource( shader, 1, pszShaderSource, iShaderSourceStringLengths );
	glCompileShader( shader );

	CheckShaderError( shader, GL_COMPILE_STATUS, false, "Compiler Error" );

	return shader;
}

ShaderGL::ShaderGL( const std::string &strShaderName )
{
	m_strShaderName = strShaderName;
}

ShaderGL::~ShaderGL()
{
}

void ShaderGL::Initialize()
{
	std::vector < char > vertexShader = ReadBinaryShader( GetName(), VERTEX_SHADER );
	std::vector < char > fragmentShader = ReadBinaryShader( GetName(), FRAGMENT_SHADER );

	GLuint hVertexShader = glCreateShader( GL_VERTEX_SHADER );

	glShaderBinary( 1, &hVertexShader, GL_SHADER_BINARY_FORMAT_SPIR_V, vertexShader.data(), ( GLsizei )vertexShader.size() );

	std::string vsEntryPoint = "main";
	glSpecializeShader( hVertexShader, ( const GLchar* )vsEntryPoint.c_str(), 0, nullptr, nullptr );

	CheckShaderError( hVertexShader, GL_COMPILE_STATUS, false, "Compiler Error" );

	GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

	glShaderBinary( 1, &hFragmentShader, GL_SHADER_BINARY_FORMAT_SPIR_V, fragmentShader.data(), ( GLsizei )fragmentShader.size()  );

	std::string fsEntryPoint = "main";
	glSpecializeShader( hFragmentShader, ( GLchar* )fsEntryPoint.c_str(), 0, nullptr, nullptr );

	CheckShaderError( hFragmentShader, GL_COMPILE_STATUS, false, "Compiler Error" );

	m_iShaders[ VERTEX_SHADER ] = hVertexShader;
	m_iShaders[ FRAGMENT_SHADER ] = hFragmentShader;

	m_hProgram = glCreateProgram();

	for( unsigned int i = 0; i < NUM_SHADERS; i++ )
        glAttachShader( m_hProgram, m_iShaders[i] );

	//GLint isCompiled = 0;


	/*m_iProgram = glCreateProgram();

	std::string strVertexShader = ReadShader( GetName(), VERTEX_SHADER );
	std::string strFragmentShader = ReadShader( GetName(), FRAGMENT_SHADER );

	m_iShaders[ VERTEX_SHADER ] = CreateShader( strVertexShader, GL_VERTEX_SHADER );
	m_iShaders[ FRAGMENT_SHADER ] = CreateShader( strFragmentShader, GL_FRAGMENT_SHADER );

	for( unsigned int i = 0; i < NUM_SHADERS; i++ )
        glAttachShader( m_iProgram, m_iShaders[i] );*/
}

const std::string &ShaderGL::GetName()
{
	return m_strShaderName;
}

unsigned int ShaderGL::GetIndex()
{
	return m_iShaderIndex;
}

std::vector < MaterialParameter_t > ShaderGL::GetMaterialParameters()
{
	return m_vMaterialParameters;
}

void ShaderGL::SetMatrix3f( const std::string &uniform, Matrix3f mat3x3 )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( mat3x3 ), &mat3x3[0][0] );
}

void ShaderGL::SetMatrix4f( const std::string &uniform, Matrix4f mat4x4 )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( mat4x4 ), &mat4x4[0][0] );
}

void ShaderGL::SetVector2f( const std::string &uniform, Vector2f vec2 )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( vec2 ), &vec2[0] );
}

void ShaderGL::SetVector3f( const std::string &uniform, Vector3f vec3 )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( vec3 ), &vec3[0] );
}

void ShaderGL::SetVector4f( const std::string &uniform, Vector4f vec4 )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( vec4 ), &vec4[0] );
}

void ShaderGL::SetFloat( const std::string &uniform, float flValue )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( flValue ), &flValue );
}

void ShaderGL::SetInt( const std::string &uniform, int iValue )
{
	assert( !m_strCurrentUBO.empty() );
	glBufferSubData( GL_UNIFORM_BUFFER, m_mapUniformOffsets[ m_strCurrentUBO + "." + uniform ], sizeof( iValue ), &iValue );
}

void ShaderGL::SetSampler( const std::string &uniform, int iValue )
{
	glUniform1iv( m_mapUniforms[ uniform ], 1, &iValue );
}

void ShaderGL::BindShader()
{
	glUseProgram( m_hProgram );
}

void ShaderGL::Shutdown()
{
	for ( auto &kV : m_mapUBOHandles )
		glDeleteBuffers( 1, &kV.second );

	for ( unsigned int i = 0; i < NUM_SHADERS; i++ )
	{
		glDetachShader( m_hProgram, m_iShaders[ i ] );
		glDeleteShader( m_iShaders[ i ] );
	}

	glDeleteProgram( m_hProgram );
}

void ShaderGL::CreateUniformBlock( const std::string &strBlockName )
{
	GLuint blockIndex = glGetUniformBlockIndex( m_hProgram, ( const GLchar* )strBlockName.c_str() );
	GLint blockSize = 0;
	GLuint uboHandle = 0;

	static GLuint blockBinding = 1;

	glGetActiveUniformBlockiv( m_hProgram, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize );

	glUniformBlockBinding( m_hProgram, blockIndex, blockBinding );
	glGenBuffers( 1, &uboHandle );
	glBindBuffer( GL_UNIFORM_BUFFER, uboHandle );
	glBufferData( GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, blockBinding, uboHandle );

	glBindBuffer( GL_UNIFORM_BUFFER, 0 );

	m_mapUBOHandles[ strBlockName ] = uboHandle;

	blockBinding++;
}

void ShaderGL::AddUniformToBlock( const std::string &strBlockName, const std::string &strUniformName )
{
	GLuint index = 0;

	std::string fullUniformName = strBlockName + "." + strUniformName;
	const GLchar *pszFullUniformName = fullUniformName.c_str();

	glGetUniformIndices( m_hProgram, 1, &pszFullUniformName, &index );

	GLint offset = 0;
	glGetActiveUniformsiv( m_hProgram, 1, &index, GL_UNIFORM_OFFSET, &offset );

	m_mapUniformOffsets[ fullUniformName ] = offset;
}

void ShaderGL::AddUniform( const std::string &strUniformName )
{
	GLint iUniformLocation = glGetUniformLocation( m_hProgram, strUniformName.c_str() );

	if ( iUniformLocation == -1 )
	{
		printf( "%s: could not find uniform: %s\n", GetName().c_str(), strUniformName.c_str() );
	}

	m_mapUniforms[ strUniformName ] = iUniformLocation;
}

void ShaderGL::BindUBO( const std::string &strBlockName )
{
	m_strCurrentUBO = strBlockName;
	glBindBuffer( GL_UNIFORM_BUFFER, m_mapUBOHandles[ strBlockName ] );
}

void ShaderGL::UnbindUBO()
{
	m_strCurrentUBO.clear();
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}

void ShaderGL::LinkAndValidate()
{
	glLinkProgram( m_hProgram );
	CheckShaderError( m_hProgram, GL_LINK_STATUS, true, "Linker Error" );

	glValidateProgram( m_hProgram );
	CheckShaderError( m_hProgram, GL_VALIDATE_STATUS, true, "Validation Error" );
}
