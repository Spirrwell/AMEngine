#version 460

// Index of the framebuffer to output to
layout( location = 0 ) out vec4 outColor;

layout( location = 0 ) in vec3 fragColor;

void main()
{
	outColor = vec4( fragColor, 1.0 );
}