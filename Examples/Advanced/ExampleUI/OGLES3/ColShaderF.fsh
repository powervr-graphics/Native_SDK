#version 300 es
uniform lowp vec4 vRGBA;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	oColour = vRGBA;
}