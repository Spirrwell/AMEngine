#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

//uniform mat4 transform;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 normalMatrix;

out vec2 texCoord0;
out vec3 normal0;
out vec3 FragPos;

void main ()
{	
	FragPos = vec3( model * vec4( position, 1.0 ) );
	texCoord0 = texCoord;
	//normal0 = mat3( transpose ( inverse ( view * model ) ) ) * normal;
	normal0 = mat3( normalMatrix ) * normal;
	gl_Position = projection * view * vec4( FragPos, 1.0 );
	//normal0 = normal;
	//color0 = color;
}