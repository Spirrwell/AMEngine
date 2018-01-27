#version 460

layout (location = 0) in vec3 position;

layout (location = 0) out vec3 texCoords;

layout (binding = 0) uniform MatrixBlock
{
	mat4 view;
	mat4 projection;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	texCoords = position;
	vec4 pos = ubo.projection * ubo.view * vec4( position, 1.0 );
	gl_Position = pos.xyww;
}