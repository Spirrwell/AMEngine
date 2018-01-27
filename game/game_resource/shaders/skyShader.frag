#version 460

out vec4 FragColor;

in vec3 texCoords;

layout (binding = 0) uniform samplerCube skybox;

void main()
{
	FragColor = texture( skybox, texCoords );
}