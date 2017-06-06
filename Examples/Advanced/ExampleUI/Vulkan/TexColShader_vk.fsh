#version 450

layout(set = 1, binding = 0)uniform sampler2D Texture;
layout(set = 2, binding = 0)uniform Color
{
	lowp vec4 vRGBA;
};

layout(location = 0) in highp vec2 TexCoord;
layout(location = 0) out lowp vec4 oColor;

void main()
{
	oColor = texture(Texture, TexCoord) * vRGBA;
}