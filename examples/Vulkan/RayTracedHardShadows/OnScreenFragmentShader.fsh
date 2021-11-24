#version 450

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 oColor;

layout(set = 0, binding = 0) uniform sampler2D RTImage;
void main()
{
	oColor = texture(RTImage, vTexCoord);
}