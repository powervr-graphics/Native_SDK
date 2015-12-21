#version 300 es
uniform lowp vec4 vRGBA;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	oColor = vRGBA;
}