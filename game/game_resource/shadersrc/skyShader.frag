#version 460

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 texCoords;

layout (binding = 0) uniform samplerCube skybox;

void main()
{
	FragColor = texture( skybox, texCoords );
}