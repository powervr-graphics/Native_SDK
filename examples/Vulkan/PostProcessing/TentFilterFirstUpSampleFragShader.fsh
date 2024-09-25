#version 320 es

layout(set = 0, binding = 0) uniform highp sampler2D sDownsampledImage;

layout(location = 0) in highp vec2 vTexCoord;
layout(location = 0) out highp float oColor;

void main()
{
	oColor = texture(sDownsampledImage, vTexCoord).r;
}