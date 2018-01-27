#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 view;
	mat4 model;
	mat4 normalMatrix;
} ubo;

layout (location = 0) out vec2 texCoord0;
layout (location = 1) out vec3 normal0;
layout (location = 2) out vec3 FragPos;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main ()
{	
	FragPos = vec3( ubo.model * vec4( position, 1.0 ) );
	texCoord0 = texCoord;
	normal0 = mat3( ubo.normalMatrix ) * normal;
	gl_Position = ubo.projection * ubo.view * vec4( FragPos, 1.0 );
}