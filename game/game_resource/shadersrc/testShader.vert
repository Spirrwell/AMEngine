#version 460

layout( binding = 0 ) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 mvp;
} ubo;

layout ( location = 0 ) in vec3 inPosition;
layout ( location = 1 ) in vec3 inColor;
layout ( location = 2 ) in vec2 inTexCoord;
layout ( location = 3 ) in vec3 inNormal;

layout ( location = 0 ) out vec3 fragColor;
layout ( location = 1 ) out vec2 fragTexCoord;

out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	//gl_Position = vec4( inPosition, 0.0, 1.0 );
	gl_Position = ubo.mvp * vec4( inPosition, 1.0 );
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}