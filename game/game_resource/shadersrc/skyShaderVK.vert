#version 460

layout (location = 0) in vec3 position;
layout (location = 0) out vec3 texCoords;

layout( push_constant ) uniform PER_OBJECT
{
	mat4 view;
	mat4 projection;
} obj;

void main()
{
	texCoords = position;
	vec4 pos = obj.projection * obj.view * vec4( position, 1.0 );
	gl_Position = pos.xyww;
}